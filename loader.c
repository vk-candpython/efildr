/*=====================================*/
// [ OWNER ]
//     CREATOR  : Vladislav Khudash
//     AGE      : 17
//     LOCATION : Ukraine
//
// [ PINFO ]
//     DATE     : 01.08.2026
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




/* Compile-time validation of feature flags */

#define _FLAG_IS_BOOLEAN(flg) _Static_assert(              \
    ((flg) == 0) || ((flg) == 1),                          \
    "Build flag: '" #flg "', must be either (0) or (1)"    \
)


_FLAG_IS_BOOLEAN(  USING_ANTI_VM           );
_FLAG_IS_BOOLEAN(  USING_ANTI_DEBUG        );
_FLAG_IS_BOOLEAN(  USING_ERASE_PE_HEADERS  );


#undef _FLAG_IS_BOOLEAN




DEC_FUNC(BOOLEAN) FindSubString(const RESTR_PTR(CHAR16) String,
                                const RESTR_PTR(CHAR16) SubString) {
    IF_UNLIKE (!(String && SubString)) return FALSE;

    const CHAR16 first = ASCII_TOLOWER(*SubString);
    IF_UNLIKE (!first) return TRUE;


    WHILE_LIKE (*String++) {
        IF_LIKE (ASCII_TOLOWER(String[-1]) != first)
            continue;

        const CHAR16 *s = String;
        const CHAR16 *p = SubString + 1;

        WHILE_LIKE (TRUE) {
            const CHAR16 a = *s;
            const CHAR16 b = *p;

            IF_UNLIKE (!(a && b) || (ASCII_TOLOWER(a) != ASCII_TOLOWER(b)))
                break;

            ++s, ++p;
        }
        IF_LIKE (!*p) return TRUE;
    }

    return FALSE;
}




/* Anti-VM Engine:
   Returns TRUE if virtual machine is DETECTED */
#if (USING_ANTI_VM)
DEC_FUNC(BOOLEAN) AntiVM(const EFI_HANDLE ImageHandle) {
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




/*
/==================\
 LOADER-ENTRY-POINT
/==================\
*/
EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable
) {
    InitializeLib(ImageHandle, SystemTable);


    Print(L"Hello World!\n\r");


    if (AntiVM(ImageHandle)) Print(L"!!!Detect-VM!!!\n\r");


    Print(L"GOODBYE!\n\r");


    BREAKPOINT();
    return EFI_SUCCESS;
}
