/*
 * Date      : 20.11.2026
 * Author    : Vladislav Khudash (18 y.o., Ukraine)
 * Source    : https://github.com/vk-candpython/efildr/blob/main/uefidef.h
 * Compiler  : MinGW-w64
 * Platform  : x86_64
 * Target    : UEFI x86_64 (PE32+)
 * Summary   : UEFI standalone declarations and basic utilities.
 * Note      : Adapted from TianoCore EDK2 reference definitions.
 * Reference : UEFI Specification (Version: 2.11)
 *             https://uefi.org/specifications
 *             EDK2: https://github.com/tianocore/edk2/tree/master/MdePkg/Include
*/


#pragma once

#ifndef EFILDR_X86_64_UEFIDEF_H
#define EFILDR_X86_64_UEFIDEF_H


///========================================///
/// *-----[COMPILATION REQUIREMENTS]-----* ///
///========================================///


#if !defined(__MINGW64__)
    #error "uefidef.h: MinGW-w64 compiler is required."
#endif


#if !defined(__x86_64__)
    #error "uefidef.h: x86_64 architecture is required."
#endif


#if defined(__STRICT_ANSI__)
    #error "uefidef.h: GNU extensions required. (use -std=gnu11)"
#endif

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)
    #error "uefidef.h: C11 standard or higher is required. (use -std=gnu11)"
#endif


///====================///
/// *-----[TYPE]-----* ///
///====================///


typedef void VOID;


typedef signed char INT8;
typedef short       INT16;
typedef int         INT32;
typedef long long   INT64;

typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;
typedef unsigned long long UINT64;

typedef INT64  INTN;
typedef UINT64 UINTN;


typedef UINT8 BOOLEAN;

typedef char   CHAR8;
typedef UINT16 CHAR16;


typedef UINTN EFI_STATUS;

typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT64 EFI_VIRTUAL_ADDRESS;

typedef UINTN  EFI_TPL;
typedef UINT64 EFI_LBA;


typedef VOID *EFI_HANDLE;
typedef VOID *EFI_EVENT;


///=====================///
/// *-----[MACRO]-----* ///
///=====================///


#define EFIAPI __attribute__((ms_abi))


#define IN
#define OUT
#define OPTIONAL


#define NULL ((VOID*)0)

#define FALSE ((BOOLEAN)(0 == 1))
#define TRUE  ((BOOLEAN)(1 != 0))


#define IMAGE_SUBSYSTEM_EFI_APPLICATION 10


#define EFI_BACKGROUND_BLACK 0x00
#define EFI_LIGHTRED         0x0C


#define EFI_OPEN_PROTOCOL_GET_PROTOCOL 0x00000002


#define EFI_FILE_MODE_READ 0x0000000000000001ULL


#define EFI_PAGE_SIZE  0x1000
#define EFI_PAGE_MASK  0xFFF
#define EFI_PAGE_SHIFT 12

#define EFI_SIZE_TO_PAGES(Size) \
    (((Size) + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT)


#define MAX_BIT 0x8000000000000000ULL

#define ENCODE_ERROR(StatusCode) \
    ((EFI_STATUS)((StatusCode) | MAX_BIT))

#define EFI_SUCCESS       ((EFI_STATUS)0)
#define EFI_LOAD_ERROR    ENCODE_ERROR(1)
#define EFI_ACCESS_DENIED ENCODE_ERROR(15)


#define uefi_call_wrapper(func, numarg, ...) \
    ((func)(__VA_ARGS__))


///=======================///
/// *-----[FORWARD]-----* ///
///=======================///


/* Forward declarations for protocol and table
   self-references prior to definition. */


/* Placeholder for unused service table entries to preserve ABI layout. */
typedef VOID (EFIAPI *EFI_UNUSED_SERVICE)(VOID);


typedef struct EFI_SYSTEM_TABLE     EFI_SYSTEM_TABLE;
typedef struct EFI_RUNTIME_SERVICES EFI_RUNTIME_SERVICES;
typedef struct EFI_BOOT_SERVICES    EFI_BOOT_SERVICES;

typedef struct _EFI_LOADED_IMAGE_PROTOCOL       EFI_LOADED_IMAGE_PROTOCOL;
typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL  EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef struct _EFI_FILE_PROTOCOL               EFI_FILE_PROTOCOL;
typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
typedef struct _EFI_PCI_IO_PROTOCOL             EFI_PCI_IO_PROTOCOL;


///====================///
/// *-----[ENUM]-----* ///
///====================///


typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown,
    EfiResetPlatformSpecific
} EFI_RESET_TYPE;

typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

typedef enum {
    EfiPciIoWidthUint8,
    EfiPciIoWidthUint16,
    EfiPciIoWidthUint32,
    EfiPciIoWidthUint64,
    EfiPciIoWidthFifoUint8,
    EfiPciIoWidthFifoUint16,
    EfiPciIoWidthFifoUint32,
    EfiPciIoWidthFifoUint64,
    EfiPciIoWidthFillUint8,
    EfiPciIoWidthFillUint16,
    EfiPciIoWidthFillUint32,
    EfiPciIoWidthFillUint64,
    EfiPciIoWidthMaximum
} EFI_PCI_IO_PROTOCOL_WIDTH;


///======================///
/// *-----[STRUCT]-----* ///
///======================///


typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
} EFI_GUID;

typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef struct {
    UINT8 Type;
    UINT8 SubType;
    UINT8 Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct {
    EFI_DEVICE_PATH_PROTOCOL Header;
    CHAR16                   PathName[1];
} FILEPATH_DEVICE_PATH;

typedef struct {
    UINT16 Year;
    UINT8  Month;
    UINT8  Day;
    UINT8  Hour;
    UINT8  Minute;
    UINT8  Second;
    UINT8  Pad1;
    UINT32 Nanosecond;
    INT16  TimeZone;
    UINT8  Daylight;
    UINT8  Pad2;
} EFI_TIME;

typedef struct {
    UINT64   Size;
    UINT64   FileSize;
    UINT64   PhysicalSize;
    EFI_TIME CreateTime;
    EFI_TIME LastAccessTime;
    EFI_TIME ModificationTime;
    UINT64   Attribute;
    CHAR16   FileName[1];
} EFI_FILE_INFO;

typedef struct {
    EFI_GUID VendorGuid;
    VOID    *VendorTable;
} EFI_CONFIGURATION_TABLE;


///============================///
/// *-----[LOADED IMAGE]-----* ///
///============================///


typedef EFI_STATUS (EFIAPI *EFI_IMAGE_UNLOAD)(
    IN EFI_HANDLE ImageHandle
);


struct _EFI_LOADED_IMAGE_PROTOCOL {
    UINT32                    Revision;
    EFI_HANDLE                ParentHandle;
    EFI_SYSTEM_TABLE         *SystemTable;
    EFI_HANDLE                DeviceHandle;
    EFI_DEVICE_PATH_PROTOCOL *FilePath;
    VOID                     *Reserved;
    UINT32                    LoadOptionsSize;
    VOID                     *LoadOptions;
    VOID                     *ImageBase;
    UINT64                    ImageSize;
    EFI_MEMORY_TYPE           ImageCodeType;
    EFI_MEMORY_TYPE           ImageDataType;
    EFI_IMAGE_UNLOAD          Unload;
};


///===========================///
/// *-----[TEXT OUTPUT]-----* ///
///===========================///


typedef struct {
    INT32   MaxMode;
    INT32   Mode;
    INT32   Attribute;
    INT32   CursorColumn;
    INT32   CursorRow;
    BOOLEAN CursorVisible;
} EFI_SIMPLE_TEXT_OUTPUT_MODE;


typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN BOOLEAN                          ExtendedVerification
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN CHAR16                          *String
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_TEST_STRING)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN CHAR16                          *String
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_QUERY_MODE)(
    IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN  UINTN                            ModeNumber,
    OUT UINTN                           *Columns,
    OUT UINTN                           *Rows
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_MODE)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN UINTN                            ModeNumber
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN UINTN                            Attribute
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_CURSOR_POSITION)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN UINTN                            Column,
    IN UINTN                            Row
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_ENABLE_CURSOR)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN BOOLEAN                          Visible
);


struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET               Reset;
    EFI_TEXT_STRING              OutputString;
    EFI_TEXT_TEST_STRING         TestString;
    EFI_TEXT_QUERY_MODE          QueryMode;
    EFI_TEXT_SET_MODE            SetMode;
    EFI_TEXT_SET_ATTRIBUTE       SetAttribute;
    EFI_TEXT_CLEAR_SCREEN        ClearScreen;
    EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
    EFI_TEXT_ENABLE_CURSOR       EnableCursor;
    EFI_SIMPLE_TEXT_OUTPUT_MODE *Mode;
};


///===========================///
/// *-----[FILE SYSTEM]-----* ///
///===========================///


typedef struct {
    EFI_EVENT  Event;
    EFI_STATUS Status;
    UINTN      BufferSize;
    VOID      *Buffer;
} EFI_FILE_IO_TOKEN;


typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN)(
    IN  EFI_FILE_PROTOCOL  *This,
    OUT EFI_FILE_PROTOCOL **NewHandle,
    IN  CHAR16             *FileName,
    IN  UINT64              OpenMode,
    IN  UINT64              Attributes
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE)(
    IN EFI_FILE_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_DELETE)(
    IN EFI_FILE_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_READ)(
    IN     EFI_FILE_PROTOCOL *This,
    IN OUT UINTN             *BufferSize,
    OUT    VOID              *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE)(
    IN     EFI_FILE_PROTOCOL *This,
    IN OUT UINTN             *BufferSize,
    IN     VOID              *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_POSITION)(
    IN  EFI_FILE_PROTOCOL *This,
    OUT UINT64            *Position
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_POSITION)(
    IN EFI_FILE_PROTOCOL *This,
    IN UINT64             Position
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_INFO)(
    IN     EFI_FILE_PROTOCOL *This,
    IN     EFI_GUID          *InformationType,
    IN OUT UINTN             *BufferSize,
    OUT    VOID              *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_INFO)(
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID          *InformationType,
    IN UINTN              BufferSize,
    IN VOID              *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH)(
    IN EFI_FILE_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN_EX)(
    IN     EFI_FILE_PROTOCOL  *This,
    OUT    EFI_FILE_PROTOCOL **NewHandle,
    IN     CHAR16             *FileName,
    IN     UINT64              OpenMode,
    IN     UINT64              Attributes,
    IN OUT EFI_FILE_IO_TOKEN  *Token
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_READ_EX)(
    IN     EFI_FILE_PROTOCOL *This,
    IN OUT EFI_FILE_IO_TOKEN *Token
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE_EX)(
    IN     EFI_FILE_PROTOCOL *This,
    IN OUT EFI_FILE_IO_TOKEN *Token
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH_EX)(
    IN     EFI_FILE_PROTOCOL *This,
    IN OUT EFI_FILE_IO_TOKEN *Token
);


struct _EFI_FILE_PROTOCOL {
    UINT64                Revision;
    EFI_FILE_OPEN         Open;
    EFI_FILE_CLOSE        Close;
    EFI_FILE_DELETE       Delete;
    EFI_FILE_READ         Read;
    EFI_FILE_WRITE        Write;
    EFI_FILE_GET_POSITION GetPosition;
    EFI_FILE_SET_POSITION SetPosition;
    EFI_FILE_GET_INFO     GetInfo;
    EFI_FILE_SET_INFO     SetInfo;
    EFI_FILE_FLUSH        Flush;
    EFI_FILE_OPEN_EX      OpenEx;
    EFI_FILE_READ_EX      ReadEx;
    EFI_FILE_WRITE_EX     WriteEx;
    EFI_FILE_FLUSH_EX     FlushEx;
};


typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(
    IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL              **Root
);

struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64                                      Revision;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
};


///=======================///
/// *-----[PCI I/O]-----* ///
///=======================///


typedef EFI_STATUS (EFIAPI *EFI_PCI_IO_PROTOCOL_CONFIG)(
    IN     EFI_PCI_IO_PROTOCOL      *This,
    IN     EFI_PCI_IO_PROTOCOL_WIDTH Width,
    IN     UINT32                    Offset,
    IN     UINTN                     Count,
    IN OUT VOID                     *Buffer
);


typedef struct {
    EFI_UNUSED_SERVICE Read;
    EFI_UNUSED_SERVICE Write;
} EFI_PCI_IO_PROTOCOL_ACCESS;

typedef struct {
    EFI_PCI_IO_PROTOCOL_CONFIG Read;
    EFI_PCI_IO_PROTOCOL_CONFIG Write;
} EFI_PCI_IO_PROTOCOL_CONFIG_ACCESS;


struct _EFI_PCI_IO_PROTOCOL {
    EFI_UNUSED_SERVICE                PollMem;
    EFI_UNUSED_SERVICE                PollIo;
    EFI_PCI_IO_PROTOCOL_ACCESS        Mem;
    EFI_PCI_IO_PROTOCOL_ACCESS        Io;
    EFI_PCI_IO_PROTOCOL_CONFIG_ACCESS Pci;
    EFI_UNUSED_SERVICE                CopyMem;
    EFI_UNUSED_SERVICE                Map;
    EFI_UNUSED_SERVICE                Unmap;
    EFI_UNUSED_SERVICE                AllocateBuffer;
    EFI_UNUSED_SERVICE                FreeBuffer;
    EFI_UNUSED_SERVICE                Flush;
    EFI_UNUSED_SERVICE                GetLocation;
    EFI_UNUSED_SERVICE                Attributes;
    EFI_UNUSED_SERVICE                GetBarAttributes;
    EFI_UNUSED_SERVICE                SetBarAttributes;
    UINT64                            RomSize;
    VOID                             *RomImage;
};


///================================///
/// *-----[RUNTIME SERVICES]-----* ///
///================================///


typedef VOID (EFIAPI *EFI_RESET_SYSTEM)(
    IN EFI_RESET_TYPE ResetType,
    IN EFI_STATUS     ResetStatus,
    IN UINTN          DataSize,
    IN VOID          *ResetData OPTIONAL
);


struct EFI_RUNTIME_SERVICES {
    EFI_TABLE_HEADER Hdr;

    EFI_UNUSED_SERVICE GetTime;
    EFI_UNUSED_SERVICE SetTime;
    EFI_UNUSED_SERVICE GetWakeupTime;
    EFI_UNUSED_SERVICE SetWakeupTime;

    EFI_UNUSED_SERVICE SetVirtualAddressMap;
    EFI_UNUSED_SERVICE ConvertPointer;

    EFI_UNUSED_SERVICE GetVariable;
    EFI_UNUSED_SERVICE GetNextVariableName;
    EFI_UNUSED_SERVICE SetVariable;

    EFI_UNUSED_SERVICE GetNextHighMonotonicCount;
    EFI_RESET_SYSTEM   ResetSystem;

    EFI_UNUSED_SERVICE UpdateCapsule;
    EFI_UNUSED_SERVICE QueryCapsuleCapabilities;

    EFI_UNUSED_SERVICE QueryVariableInfo;
};


///=============================///
/// *-----[BOOT SERVICES]-----* ///
///=============================///


typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_PAGES)(
    IN     EFI_ALLOCATE_TYPE     Type,
    IN     EFI_MEMORY_TYPE       MemoryType,
    IN     UINTN                 Pages,
    IN OUT EFI_PHYSICAL_ADDRESS *Memory
);

typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)(
    IN  EFI_MEMORY_TYPE PoolType,
    IN  UINTN           Size,
    OUT VOID          **Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL)(
    IN VOID *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_HANDLE_PROTOCOL)(
    IN  EFI_HANDLE Handle,
    IN  EFI_GUID  *Protocol,
    OUT VOID     **Interface
);

typedef EFI_STATUS (EFIAPI *EFI_STALL)(
    IN UINTN Microseconds
);

typedef VOID (EFIAPI *EFI_COPY_MEM)(
    IN VOID *Destination,
    IN VOID *Source,
    IN UINTN Length
);

typedef VOID (EFIAPI *EFI_SET_MEM)(
    IN VOID *Buffer,
    IN UINTN Size,
    IN UINT8 Value
);

typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL)(
    IN  EFI_HANDLE Handle,
    IN  EFI_GUID  *Protocol,
    OUT VOID     **Interface OPTIONAL,
    IN  EFI_HANDLE AgentHandle,
    IN  EFI_HANDLE ControllerHandle,
    IN  UINT32     Attributes
);

typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE_BUFFER)(
    IN  EFI_LOCATE_SEARCH_TYPE SearchType,
    IN  EFI_GUID              *Protocol  OPTIONAL,
    IN  VOID                  *SearchKey OPTIONAL,
    OUT UINTN                 *NoHandles,
    OUT EFI_HANDLE           **Buffer
);


struct EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr;

    EFI_UNUSED_SERVICE RaiseTPL;
    EFI_UNUSED_SERVICE RestoreTPL;

    EFI_ALLOCATE_PAGES AllocatePages;
    EFI_UNUSED_SERVICE FreePages;
    EFI_UNUSED_SERVICE GetMemoryMap;
    EFI_ALLOCATE_POOL  AllocatePool;
    EFI_FREE_POOL      FreePool;

    EFI_UNUSED_SERVICE CreateEvent;
    EFI_UNUSED_SERVICE SetTimer;
    EFI_UNUSED_SERVICE WaitForEvent;
    EFI_UNUSED_SERVICE SignalEvent;
    EFI_UNUSED_SERVICE CloseEvent;
    EFI_UNUSED_SERVICE CheckEvent;

    EFI_UNUSED_SERVICE  InstallProtocolInterface;
    EFI_UNUSED_SERVICE  ReinstallProtocolInterface;
    EFI_UNUSED_SERVICE  UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL HandleProtocol;
    VOID               *Reserved;
    EFI_UNUSED_SERVICE  RegisterProtocolNotify;
    EFI_UNUSED_SERVICE  LocateHandle;
    EFI_UNUSED_SERVICE  LocateDevicePath;
    EFI_UNUSED_SERVICE  InstallConfigurationTable;

    EFI_UNUSED_SERVICE LoadImage;
    EFI_UNUSED_SERVICE StartImage;
    EFI_UNUSED_SERVICE Exit;
    EFI_UNUSED_SERVICE UnloadImage;
    EFI_UNUSED_SERVICE ExitBootServices;

    EFI_UNUSED_SERVICE GetNextMonotonicCount;
    EFI_STALL          Stall;
    EFI_UNUSED_SERVICE SetWatchdogTimer;

    EFI_UNUSED_SERVICE ConnectController;
    EFI_UNUSED_SERVICE DisconnectController;

    EFI_OPEN_PROTOCOL  OpenProtocol;
    EFI_UNUSED_SERVICE CloseProtocol;
    EFI_UNUSED_SERVICE OpenProtocolInformation;

    EFI_UNUSED_SERVICE       ProtocolsPerHandle;
    EFI_LOCATE_HANDLE_BUFFER LocateHandleBuffer;
    EFI_UNUSED_SERVICE       LocateProtocol;
    EFI_UNUSED_SERVICE       InstallMultipleProtocolInterfaces;
    EFI_UNUSED_SERVICE       UninstallMultipleProtocolInterfaces;

    EFI_UNUSED_SERVICE CalculateCrc32;

    EFI_COPY_MEM       CopyMem;
    EFI_SET_MEM        SetMem;
    EFI_UNUSED_SERVICE CreateEventEx;
};


///============================///
/// *-----[SYSTEM TABLE]-----* ///
///============================///


struct EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER Hdr;

    CHAR16 *FirmwareVendor;
    UINT32  FirmwareRevision;

    EFI_HANDLE                       ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *ConIn;
    EFI_HANDLE                       ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                       StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;

    EFI_RUNTIME_SERVICES *RuntimeServices;
    EFI_BOOT_SERVICES    *BootServices;

    UINTN                    NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE *ConfigurationTable;
};


///====================///
/// *-----[GUID]-----* ///
///====================///


#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
    {0x5B1B31A1, 0x9562, 0x11D2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}}

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
    {0x964E5B22, 0x6459, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}}

#define EFI_FILE_INFO_ID \
    {0x09576E92, 0x6D3F, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}}

#define EFI_PCI_IO_PROTOCOL_GUID \
    {0x4CF5B200, 0x68B8, 0x4CA5, {0x9E, 0xEC, 0xB2, 0x3E, 0x3F, 0x50, 0x02, 0x9A}}


///======================///
/// *-----[GLOBAL]-----* ///
///======================///


static EFI_SYSTEM_TABLE     *ST;
static EFI_RUNTIME_SERVICES *RT;
static EFI_BOOT_SERVICES    *BS;


///=======================///
/// *-----[UTILITY]-----* ///
///=======================///


#define InitializeLib(ImageHandle, SystemTable) do {    \
    (VOID)(ImageHandle);                                \
    ST = SystemTable;                                   \
    RT = SystemTable->RuntimeServices;                  \
    BS = SystemTable->BootServices;                     \
} while (0)


#define CopyMem(dst, src, sz) \
    (BS->CopyMem((VOID*)(dst), (VOID*)(src), (UINTN)(sz)))

#define ZeroMem(dst, sz) \
    (BS->SetMem((VOID*)(dst), (UINTN)(sz), (UINT8)(0)))


#define StrLen(String) ({                             \
    const CHAR16 *_StR = (String);                    \
    const CHAR16 *_PtR = _StR;                        \
                                                      \
    while (__builtin_expect(!!(*_PtR), 1)) ++_PtR;    \
                                                      \
    (UINTN)(_PtR - _StR);                             \
})


#endif /* EFILDR_X86_64_UEFIDEF_H */
