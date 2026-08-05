/*
 * Author   : Vladislav Khudash
 * Source   : https://github.com/vk-candpython/efildr/blob/main/loader.h
 * Compiler : GCC (GNU-EFI)
 * Platform : UEFI (PE32+)
 * Summary  : Core configuration header, custom NT structures, and basic utilities.
*/


#pragma once


/********************
 *     INCLUDES     *
 ********************/


/* GNU-EFI headers,
   UEFI core definitions */
#include <efi.h>
#include <efilib.h>




/*************************************
 *     BUILD-TIME FEATURE CONFIG     *
 *************************************/


#define USING_ANTI_VM          0
#define USING_ANTI_DEBUG       0
#define USING_ERASE_PE_HEADERS 0




/*********************************
 *     CONSTANT-DECLARATIONS     *
 *********************************/


#define DAT_KEY_SZ sizeof(UINT8)  // Size of key size field (BYTE)
#define DAT_LEN_SZ sizeof(UINT32) // Size of length fields (UINT32)

#define RLE_MAX_RUN 127 // Maximum encoded run length
#define RLE_FLG_RUN 128 // High-bit marker for run blocks


#define LDR_REBOOT_SECOND 5 // Delay before reboot




/*************************************
 *     ERROR-STRING DECLARATIONS     *
 *************************************/


#define LDR_ERR_PREFIX L"\r\n(efildr) [ERROR]: "

#define LDR_ERR_ANALYSIS     L"Image initialization failed.\r\n"
#define LDR_ERR_SUBSYSTEM    L"Image is not an EFI application.\r\n"
#define LDR_ERR_OVERLAY      L"Image overlay is corrupted.\r\n"
#define LDR_ERR_ALLOC_IMAGE  L"Image memory allocation failed.\r\n"
#define LDR_ERR_LOADED_IMAGE L"Loaded image protocol access failed.\r\n"




/*****************************
 *     ALIASES & HELPERS     *
 *****************************/


#define _EFI_CALL_SELECTOR(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define _EFI_CALL_ARGS(...) _EFI_CALL_SELECTOR(0, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/* uefi_call_wrapper with automatic argument counting */
#define EFI_CALL(func, ...) \
    uefi_call_wrapper((func), _EFI_CALL_ARGS(__VA_ARGS__), ##__VA_ARGS__)



/* Compiler attribute wrapper helper */
#define _DEC_ATTR(...) \
    __attribute__((__VA_ARGS__))


/* Assembly block shorthand */
#define DEC_ASM \
    __asm__ volatile



/* Const-qualified pointer helper
   (const data and const pointer) */
#define CONST_PTR(TYPE) \
    const TYPE *const

/* Restrict-qualified pointer helper
   (guarantees no memory aliasing) */
#define RESTR_PTR(TYPE) \
    TYPE *restrict



/* Branch prediction hints for compiler optimization */

#define _UNLIKELY(x) __builtin_expect(!!(x), 0) // Cold path
#define _LIKELY(x)   __builtin_expect(!!(x), 1) // Hot path


/* Check for EFI status success */
#define EFI_OK(status) \
    _LIKELY((INTN)(status) >= EFI_SUCCESS)

/* Check for EFI status failure */
#define EFI_FAIL(status) \
    _UNLIKELY((INTN)(status) < EFI_SUCCESS)


/* Short-circuit EFI call chainer */

#define _EFI_CHECK_CALL_N(...)           (EFI_OK(EFI_CALL(__VA_ARGS__)))
#define _EFI_CHECK_CALL_1(a)             (_EFI_CHECK_CALL_N a)
#define _EFI_CHECK_CALL_2(a, b)          (_EFI_CHECK_CALL_1(a)          && _EFI_CHECK_CALL_1(b))
#define _EFI_CHECK_CALL_3(a, b, c)       (_EFI_CHECK_CALL_2(a, b)       && _EFI_CHECK_CALL_1(c))
#define _EFI_CHECK_CALL_4(a, b, c, d)    (_EFI_CHECK_CALL_3(a, b, c)    && _EFI_CHECK_CALL_1(d))
#define _EFI_CHECK_CALL_5(a, b, c, d, e) (_EFI_CHECK_CALL_4(a, b, c, d) && _EFI_CHECK_CALL_1(e))

#define _EFI_CHECK_CALL_SELECTOR(_1, _2, _3, _4, _5, N, ...) _EFI_CHECK_CALL_##N

/* Automatic EFI call-chain validator */
#define EFI_CHAIN_CALL(...) \
    _EFI_CHECK_CALL_SELECTOR(__VA_ARGS__, 5, 4, 3, 2, 1)(__VA_ARGS__)


/* Optimized branch control macros */

#define IF_UNLIKE(expr) if (_UNLIKELY(expr))
#define IF_LIKE(expr)   if (_LIKELY(expr))

#define IF_EFIFAIL(status)        if         (EFI_FAIL(status))
#define IF_EFIFAIL_CALL(...)      IF_EFIFAIL (EFI_CALL(__VA_ARGS__))
#define IF_EFIFAIL_CHAINCALL(...) IF_UNLIKE  (!EFI_CHAIN_CALL(__VA_ARGS__))


/* Optimized loop control macros */

#define WHILE_LIKE(expr) \
    while (_LIKELY(expr))

#define FOR_LIKE(init, cond, post) \
    for (init; _LIKELY(cond); post)



/* Function declaration helper */
#define DEC_FUNC(TYPE) \
    static inline _DEC_ATTR(always_inline) TYPE




/******************************
 *     PE-IMAGE UTILITIES     *
 ******************************/


/* ASCII-only lowercase conversion */
#define ASCII_TOLOWER(chr) \
    ((chr) | 0x20)


/* Convert seconds to microseconds */
#define SEC_TO_USEC(n) \
    ((UINTN)(n) * 1000000)


/* Safe minimum and maximum value helpers */

#define MIN(a, b) ({         \
    typeof((a)) _a = (a);    \
    typeof((b)) _b = (b);    \
                             \
    (_a < _b)? _a : _b;      \
})

#define MAX(a, b) ({         \
    typeof((a)) _a = (a);    \
    typeof((b)) _b = (b);    \
                             \
    (_a > _b)? _a : _b;      \
})



/* Convert RVA to typed pointer */
#define RVA(TYPE, base, addr) \
    ((TYPE)((UINT8*)(base) + (addr)))


/* Extract e_lfanew from DOS header */
#define DOS_LFANEW(base) \
    (((IMAGE_DOS_HEADER*)(base))->e_lfanew)


/* Get the current EFI image file path */
#define EFI_IMAGE_PATH(LoadedImage) \
    (((FILEPATH_DEVICE_PATH*)(LoadedImage)->FilePath)->PathName)

/* Calculate the required EFI_FILE_INFO buffer size,
   Uses the full path as buffer slack */
#define EFI_FILEINFO_SIZE(PathName)         \
    (sizeof(EFI_FILE_INFO) +                \
    (StrLen(PathName) * sizeof(CHAR16)))


/* Number of relocation entries in a block */
#define RELOC_ENTRY_COUNT(reloc)                              \
    (((reloc)->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)    \
     ) / sizeof(UINT16))

/* True if relocation entry requests
   64-bit absolute fixup */
#define RELOC_IS_DIR64(entry) \
    (((entry) >> 12) == IMAGE_REL_BASED_DIR64)

/* Get intra‑page offset from relocation entry */
#define RELOC_BLOCK_OFFSET(entry) \
    ((entry) & 0x0FFF)


/* Retrieves a pointer to the first section header */
#define IMAGE_FIRST_SECTION(hdNt) RVA(         \
    IMAGE_SECTION_HEADER*,                     \
    &(hdNt)->OptionalHeader,                   \
    (hdNt)->FileHeader.SizeOfOptionalHeader    \
)



/* Copy memory block */
#define MEMCPY(dst, src, sz) \
    CopyMem((VOID*)(dst), (const VOID*)(src), (UINTN)(sz))

/* Fill memory with zeros */
#define ZEROS(dst, sz) \
    ZeroMem((VOID*)(dst), (UINTN)(sz))




/*******************************
 *     ANTI-VM DEFINITIONS     *
 *******************************/


/* Common physical PC UEFI
   firmware vendor substrings */
#define IBV_FIRMWARE_VENDORS {                                        \
    L"megatrends", /* (American Megatrends International, LLC.) */    \
    L"insyde",     /* (INSYDE Corp.) */                               \
    L"phoenix",    /* (Phoenix Technologies Ltd.) */                  \
    L"byosoft",    /* (Nanjing Byosoft Co., Ltd.) */                  \
}


/* PCI vendor IDs packed as UINT16 */

#define PCI_VENID_VBOX      ((UINT16)0x80EE) // Oracle Corporation (VirtualBox)
#define PCI_VENID_VMWARE    ((UINT16)0x15AD) // VMware, Inc.

#define PCI_VENID_QEMU      ((UINT16)0x1AF4) // Red Hat (QEMU VirtIO)
#define PCI_VENID_QEMU_BRG  ((UINT16)0x1B36) // Red Hat (QEMU PCI Bridge)
#define PCI_VENID_QEMU_VGA  ((UINT16)0x1234) // QEMU Virtual VGA

#define PCI_VENID_XEN       ((UINT16)0x5853) // XenSource, Inc.
#define PCI_VENID_HYPER_V   ((UINT16)0x1414) // Microsoft Corporation (Hyper-V)
#define PCI_VENID_PARALLELS ((UINT16)0x1AB8) // Parallels International GmbH




/**********************************
 *     ANTI-DEBUG DEFINITIONS     *
 **********************************/


#define CPU_RFLAGS_TF   0x100ULL // RFLAGS Trap Flag (TF) - Bit 8
#define CPU_DR7_BP_MASK 0xFFULL  // DR7 L0-L3/G0-G3 enable bits (Bits 0-7)




/************************************
 *     NT-CONSTANT DECLARATIONS     *
 ************************************/


#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16
#define IMAGE_SIZEOF_SHORT_NAME          8


#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define IMAGE_REL_BASED_DIR64           10




/************************************
 *     NT-STRUCTURE DEFINITIONS     *
 ************************************/


typedef struct {
    UINT16    e_magic;
    UINT16    e_cblp;
    UINT16    e_cp;
    UINT16    e_crlc;
    UINT16    e_cparhdr;
    UINT16    e_minalloc;
    UINT16    e_maxalloc;
    UINT16    e_ss;
    UINT16    e_sp;
    UINT16    e_csum;
    UINT16    e_ip;
    UINT16    e_cs;
    UINT16    e_lfarlc;
    UINT16    e_ovno;
    UINT16    e_res[4];
    UINT16    e_oemid;
    UINT16    e_oeminfo;
    UINT16    e_res2[10];
    INT32     e_lfanew;
} IMAGE_DOS_HEADER;


typedef struct {
    UINT16    Machine;
    UINT16    NumberOfSections;
    UINT32    TimeDateStamp;
    UINT32    PointerToSymbolTable;
    UINT32    NumberOfSymbols;
    UINT16    SizeOfOptionalHeader;
    UINT16    Characteristics;
} IMAGE_FILE_HEADER;


typedef struct {
    UINT32    VirtualAddress;
    UINT32    Size;
} IMAGE_DATA_DIRECTORY;

typedef struct {
    UINT16    Magic;
    UINT8     MajorLinkerVersion;
    UINT8     MinorLinkerVersion;
    UINT32    SizeOfCode;
    UINT32    SizeOfInitializedData;
    UINT32    SizeOfUninitializedData;
    UINT32    AddressOfEntryPoint;
    UINT32    BaseOfCode;
    UINT64    ImageBase;
    UINT32    SectionAlignment;
    UINT32    FileAlignment;
    UINT16    MajorOperatingSystemVersion;
    UINT16    MinorOperatingSystemVersion;
    UINT16    MajorImageVersion;
    UINT16    MinorImageVersion;
    UINT16    MajorSubsystemVersion;
    UINT16    MinorSubsystemVersion;
    UINT32    Win32VersionValue;
    UINT32    SizeOfImage;
    UINT32    SizeOfHeaders;
    UINT32    CheckSum;
    UINT16    Subsystem;
    UINT16    DllCharacteristics;
    UINT64    SizeOfStackReserve;
    UINT64    SizeOfStackCommit;
    UINT64    SizeOfHeapReserve;
    UINT64    SizeOfHeapCommit;
    UINT32    LoaderFlags;
    UINT32    NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64;


typedef struct {
    UINT32                     Signature;
    IMAGE_FILE_HEADER          FileHeader;
    IMAGE_OPTIONAL_HEADER64    OptionalHeader;
} IMAGE_NT_HEADERS64;


typedef struct {
    UINT8     Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
        UINT32    PhysicalAddress;
        UINT32    VirtualSize;
    } Misc;
    UINT32    VirtualAddress;
    UINT32    SizeOfRawData;
    UINT32    PointerToRawData;
    UINT32    PointerToRelocations;
    UINT32    PointerToLinenumbers;
    UINT16    NumberOfRelocations;
    UINT16    NumberOfLinenumbers;
    UINT32    Characteristics;
} IMAGE_SECTION_HEADER;


typedef struct {
    UINT32    VirtualAddress;
    UINT32    SizeOfBlock;
} IMAGE_BASE_RELOCATION;
