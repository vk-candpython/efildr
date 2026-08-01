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


#include <efi.h>
#include <efilib.h>




/*************************************
 *     BUILD-TIME FEATURE CONFIG     *
 *************************************/


#define USING_ANTI_VM          1
#define USING_ANTI_DEBUG       1
#define USING_ERASE_PE_HEADERS 1




/*********************************
 *     CONSTANT-DECLARATIONS     *
 *********************************/


#define DAT_KEY_SZ sizeof(UINT8)  // Size of key size field (UINT8)
#define DAT_LEN_SZ sizeof(UINT32) // Size of length fields (UINT32)

#define RLE_MAX_RUN 127 // Maximum encoded run length
#define RLE_FLG_RUN 128 // High-bit marker for run blocks




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



/* Stack buffer alignment helper */
#define DEC_ALIGN_BUF \
    _DEC_ATTR(aligned(16))


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


/* Check for EFI status failure */
#define EFI_FAIL(status) \
    _UNLIKELY((INTN)(status) < 0)


/* Optimized branch control macros */

#define IF_LIKE(expr)      if (_LIKELY(expr))
#define IF_UNLIKE(expr)    if (_UNLIKELY(expr))
#define IF_EFIFAIL(status) if (EFI_FAIL(status))


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


#define _rotr8(v, n) (((v) >> (n)) | ((v) << (8 - (n))))


/* ASCII-only lowercase conversion */
#define ASCII_TOLOWER(chr) \
    ((chr) | 0x20)


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

/* Extracts PE section protection index
   from Characteristics (bits 29–31) */
#define SECTION_PROT_IDX(attr) \
    ((UINT8)(((attr) & SECTION_PROT_MASK) >> 29))



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
#define IBV_FIRMWARE_VENDORS {    \
    L"megatrends",                \
    L"insyde",                    \
    L"phoenix",                   \
    L"byosoft",                   \
}


/* PCI vendor IDs packed as UINT16 */

#define PCI_VENID_VBOX      ((UINT16)0x80EE) // "80EE"
#define PCI_VENID_VMWARE    ((UINT16)0x15AD) // "15AD"

#define PCI_VENID_QEMU      ((UINT16)0x1AF4) // "1AF4"
#define PCI_VENID_QEMU_BRG  ((UINT16)0x1B36) // "1B36"
#define PCI_VENID_QEMU_VGA  ((UINT16)0x1234) // "1234"

#define PCI_VENID_XEN       ((UINT16)0x5853) // "5853"
#define PCI_VENID_HYPER_V   ((UINT16)0x1414) // "1414"
#define PCI_VENID_PARALLELS ((UINT16)0x1AB8) // "1AB8"




/**********************************
 *     ANTI-DEBUG DEFINITIONS     *
 **********************************/




/************************************
 *     NT-CONSTANT DECLARATIONS     *
 ************************************/


#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16
#define IMAGE_SIZEOF_SHORT_NAME          8


#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define IMAGE_REL_BASED_DIR64           10


#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40


#define IMAGE_SCN_MEM_READ    0x40000000
#define IMAGE_SCN_MEM_WRITE   0x80000000
#define IMAGE_SCN_MEM_EXECUTE 0x20000000

#define SECTION_PROT_MASK \
    (IMAGE_SCN_MEM_READ|IMAGE_SCN_MEM_WRITE|IMAGE_SCN_MEM_EXECUTE)




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
    UINT32    e_lfanew;
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
