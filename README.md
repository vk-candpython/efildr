# 👾 efildr

<div align="center">

[![Platform](https://img.shields.io/badge/platform-UEFI-blue?logo=uefi&logoColor=white)](https://uefi.org)
[![Language](https://img.shields.io/badge/language-C-00599C?logo=c)](https://en.cppreference.com/)
[![Builder](https://img.shields.io/badge/builder-Python%203-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

*Compress, Encrypt, Fileless Execute — One EFI binary, zero traces.*

### 📺 [Watch demo on YouTube](https://www.youtube.com/watch?v=yT6yCbjUaeI)

</div>

---

> [!WARNING]
> This tool is intended **only** for educational purposes and authorized security auditing.
> Use it only in test environments and on systems you own or are explicitly authorized to assess.
> The author assumes no liability for misuse or damage.

---

## 📖 Table of Contents | Оглавление

- [English](#english)
  - [📋 Overview](#-overview)
  - [✨ Features](#-features)
  - [🔒 Complete Packing Pipeline](#-complete-packing-pipeline)
  - [🚀 Quick Start](#-quick-start)
  - [⚙️ Build-time Configuration Flags](#️-build-time-configuration-flags)
  - [⚙️ Technical Highlights](#️-technical-highlights)
  - [📁 Output](#-output)
  - [⚠️ Requirements](#️-requirements)
  - [🔧 Troubleshooting](#-troubleshooting)
- [Русский](#русский)
  - [📋 Обзор](#-обзор)
  - [✨ Возможности](#-возможности)
  - [🔒 Полный конвейер упаковки](#-полный-конвейер-упаковки)
  - [🚀 Быстрый старт](#-быстрый-старт)
  - [⚙️ Флаги конфигурации времени сборки](#️-флаги-конфигурации-времени-сборки)
  - [⚙️ Технические особенности](#️-технические-особенности)
  - [📁 Результат](#-результат)
  - [⚠️ Требования](#️-требования)
  - [🔧 Устранение неполадок](#-устранение-неполадок)

---

# English

## 📋 Overview

**EFILDR** is a reflective EFI loader and packer that transforms standard UEFI applications (PE32+) into self-contained, encrypted EFI binaries.
It combines RLE compression, a custom stateful byte transformation, and a reflective loader stub written in C. The stub is built as a standalone UEFI PE32+ application without the C runtime (CRT).

The loader reads its appended overlay, decrypts and decompresses the payload in memory, maps the image, applies base relocations, and transfers execution. The packed payload is not extracted as a separate file.

### Key Components

| Component | Description |
|-----------|-------------|
| **Reflective Loader** | C implementation using custom UEFI declarations and direct UEFI protocol calls |
| **ARX-style Byte Transformation** | Custom stateful byte transformation with data-dependent rotations and key/state updates |
| **RLE Compression** | Run-length encoding to reduce payload size |
| **Runtime Checks** | Optional virtualisation and debugger checks |
| **In-memory Loading** | The payload is unpacked and mapped in memory rather than written as a separate file |

## ✨ Features

### Core Packing

| Feature | Description |
|---------|-------------|
| 🗜️ **Compression** | Custom RLE encoding; the actual saving depends on the input image |
| 🔐 **Encryption** | Stateful byte transformation with feedback between successive bytes |
| 🧠 **Reflective Loading** | PE image mapping, section copying, base relocations, and image-base update |
| 🧹 **Header Erasure** | Optional removal of PE headers from the mapped image after loading |

### Compilation & Linking

| Feature | Description |
|---------|-------------|
| 🏗️ **Freestanding Build** | `-nostdlib -ffreestanding` — builds without the standard C runtime |
| 🧹 **Stripped Binary** | `-s` strips symbols from the final image |
| ⚡ **Optimised Output** | `-O3`, function/data sections, and linker section garbage collection |
| 📦 **Compact Stub** | Current loader stub is approximately **4 KB**; the exact size depends on the toolchain and build options |

### Runtime Checks

| Feature | Description |
|---------|-------------|
| 🛡️ **Anti-analysis Checks** | Optional checks for debugging and virtualised environments |
| 🔒 **Memory Cleanup** | Allocated buffers and opened file handles are released on the handled exit paths |

### Builder (Python)

| Feature | Description |
|---------|-------------|
| 🐍 **Cross-platform** | Python 3 builder with no external packages required |
| 🔑 **Variable Key Length** | Supports 16, 32, 64, or 128-byte keys |
| 📊 **Statistics** | Reports original size, compressed size, saved bytes/percentage, and elapsed time |

## 🔒 Complete Packing Pipeline

```text
PHASE 1: PE VALIDATION
├── Checks PE signatures (MZ, PE)
├── Confirms x86-64 machine type
├── Verifies PE32+ format and EFI application subsystem
└── Enforces the builder's configured size limits

PHASE 2: COMPRESSION
└── Custom run-length encoding (run blocks: 3–127 bytes)

PHASE 3: ENCRYPTION
├── Generates a key (16–128 bytes)
└── Applies the stateful byte transformation to the compressed payload

PHASE 4: STUB ASSEMBLY
├── Uses the pre-built EFI PE32+ loader stub
├── Appends the key and compressed/encrypted payload
├── Stores the overlay length for runtime parsing
└── Produces a single .efi file

PHASE 5: RUNTIME (Loader Execution)
├── Reads the overlay from its own EFI image file
├── Decrypts and decompresses the payload in memory
├── Allocates pages, copies PE headers/sections, and applies relocations
├── Updates the loaded-image protocol with the mapped base and image size
├── Applies optional runtime checks
├── Optionally erases PE headers from the mapped image
└── Transfers control to the payload entry point
```

## 🚀 Quick Start

### 📥 Get the Source

```bash
git clone https://github.com/vk-candpython/efildr.git
cd efildr
```

### 🛠️ Build the Loader

The loader is compiled directly into a **UEFI x86-64 PE32+** image. Choose one of the following toolchains. Both commands produce `loader.efi`; they are alternatives, not consecutive build steps.

#### Option A: Clang + LLD (POSIX shell)

Requires Clang and LLD. The MinGW target selects the x86-64 PE/COFF output format; the result is a UEFI application, not a regular Windows console executable.

```bash
clang -o loader.efi loader.c \
    --target=x86_64-w64-mingw32 \
    -fuse-ld=lld \
    -std=gnu11 \
    -Wall -Wextra \
    -O3 -s -g0 \
    \
    -nostdlib \
    -ffreestanding \
    -fshort-wchar \
    -mno-red-zone \
    \
    -fno-ident \
    -fno-common \
    -fdata-sections \
    -ffunction-sections \
    -fstrict-aliasing \
    -fomit-frame-pointer \
    -fvisibility=hidden \
    -fno-stack-protector \
    \
    -Wl,/machine:x64 \
    -Wl,/subsystem:efi_application \
    -Wl,/entry:efi_main \
    -Wl,/nodefaultlib \
    -Wl,/nxcompat \
    -Wl,/dynamicbase \
    -Wl,/opt:ref \
    -Wl,/opt:icf
```

#### Option B: GCC + MinGW-w64 (w64devkit64, Windows CMD)

Run this command in **Windows Command Prompt (`cmd.exe`)** with the 64-bit GCC toolchain from w64devkit64 available on `PATH`. In CMD, `^` is the line-continuation character and must be the final character on each continued line.

```cmd
gcc -o loader.efi loader.c ^
    -m64 ^
    -std=gnu11 ^
    -Wall -Wextra ^
    -O3 -s -g0 ^
    ^
    -nostdlib ^
    -ffreestanding ^
    -fshort-wchar ^
    -mno-red-zone ^
    ^
    -fno-ident ^
    -fno-common ^
    -fdata-sections ^
    -ffunction-sections ^
    -fstrict-aliasing ^
    -fomit-frame-pointer ^
    -fvisibility=hidden ^
    -fno-stack-protector ^
    ^
    -Wl,--subsystem,10 ^
    -Wl,--entry,efi_main ^
    -Wl,--nxcompat ^
    -Wl,--dynamicbase ^
    -Wl,--gc-sections
```

### 🔍 Verify the EFI Image

After building, inspect the generated PE headers and sections:

```bash
file loader.efi
objdump -p loader.efi
objdump -h loader.efi
```

Check that the output is PE32+ / AMD64, has the EFI application subsystem (`10`), uses `efi_main` as its entry point, and contains a base-relocation directory when relocations are required.

### 🐍 Run the Builder

```bash
python3 efildr.py myapp.efi
```

The builder compresses and transforms `myapp.efi`, then attaches the overlay to a copy of the loader stub, creating `./efildr-myapp.efi`.

**Output example:**

```text
Start processing  -> myapp.efi

[*] compressed    :  148992 -> 128221 bytes  |  saved: 20771 bytes (13.9%)
[+] output file   :  ./efildr-myapp.efi (164598 bytes)
[*] elapsed time  :  0.08s

End of processing -> myapp.efi
```

### 🧪 Testing with QEMU

```bash
mkdir -p disk/EFI/BOOT
cp ./efildr-myapp.efi disk/EFI/BOOT/BOOTX64.EFI
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd \
    -drive format=raw,file=fat:rw:disk -net none -m 256M
```

## ⚙️ Build-time Configuration Flags

The loader (`loader.h`) exposes three compile-time switches that control its behaviour:

```c
#define USING_ANTI_VM          0
#define USING_ANTI_DEBUG       0
#define USING_ERASE_PE_HEADERS 0
```

| Flag | Default | Description |
|------|---------|-------------|
| **`USING_ANTI_VM`** | `0` | Enables checks for virtualised environments |
| **`USING_ANTI_DEBUG`** | `0` | Enables anti-debugging checks |
| **`USING_ERASE_PE_HEADERS`** | `0` | Erases DOS/NT headers from the mapped image after loading |

Changing any flag requires rebuilding the loader stub before packing.

## ⚙️ Technical Highlights

- **Direct EFI Protocol Calls** — Boot Services and protocol functions are called through the custom `EFI_CALL` macro with automatic argument counting.
- **Short-Circuit Call Chains** — `IF_EFIFAIL_CHAINCALL` chains protocol calls and stops when a call reports an EFI error.
- **Stateful Byte Transformation** — `DEC_BYTE` performs the project's custom byte transformation and updates its internal state for the next byte. It has not been presented as a cryptographically audited cipher.
- **Reflective Loading** — the payload is mapped, relocated, and executed from memory, with the loaded-image information updated to the mapped image.
- **Optional Runtime Checks** — compile-time switches control the environment and debugger checks.
- **Memory Cleanup** — allocated buffers and opened file handles are released along the handled cleanup paths.
- **EFI File Information Buffer** — the required `EFI_FILE_INFO` buffer size is calculated from the image path before the buffer is allocated.

## 📁 Output

```text
original.efi  →  efildr-original.efi
```

- One EFI executable containing the loader and the packed payload.
- The packed payload is not stored as a separate plaintext file.
- The loader stub is approximately **4 KB** in the current build; the final packed image is larger and depends on the input payload.
- The stub is stripped in the documented release commands (`-s`).

## ⚠️ Requirements

| Requirement | Version / Tool | Notes |
|-------------|----------------|-------|
| **Loader compiler — Option A** | Clang + LLD | Target: `x86_64-w64-mingw32` |
| **Loader compiler — Option B** | GCC + MinGW-w64 | 64-bit toolchain, e.g. w64devkit64 |
| **Builder** | Python 3.6+ | No extra packages required by the builder |
| **Target platform** | UEFI x86-64 (PE32+) | EFI application subsystem (`10`) |
| **Optional testing** | QEMU + OVMF | `qemu-system-x86_64` and an OVMF firmware image |

GNU-EFI headers, `crt0-efi-x86_64.o`, and ELF-to-PE conversion are not required by the two direct PE/COFF build commands above.

## 🔧 Troubleshooting

| Issue | Solution |
|-------|----------|
| `clang: linker command failed` | Ensure LLD is installed and available to Clang (`-fuse-ld=lld`). |
| GCC does not recognise a linker option | Confirm that GCC is the MinGW-w64 build from w64devkit64 and that its PE-targeted GNU `ld` is being used. |
| `builder reports "Invalid EFI image"` | The input must be a 64-bit EFI application (PE32+, subsystem `10`). Check it with `objdump -p`. |
| QEMU boots to shell | Place the image at `disk/EFI/BOOT/BOOTX64.EFI` and verify the OVMF firmware path. |
| `overlay corrupted` error in loader | Ensure the builder and loader use matching overlay layouts and key-size conventions. |
| `file size too large` | The builder's default maximum input size is 8 MiB; adjust its configured limit if appropriate. |

---

# Русский

## 📋 Обзор

**EFILDR** — рефлективный EFI-загрузчик и упаковщик, который преобразует стандартные UEFI-приложения (PE32+) в автономные EFI-файлы с упакованной и преобразованной полезной нагрузкой.
В проекте используются RLE-сжатие, собственное потоковое преобразование байтов с состоянием и загрузчик на C, собираемый как самостоятельное UEFI-приложение PE32+ без CRT.

Загрузчик читает присоединённый overlay, распаковывает и преобразует полезную нагрузку в памяти, отображает PE-образ, применяет базовые релокации и передаёт управление точке входа. Исходная полезная нагрузка не извлекается в отдельный файл.

### Ключевые компоненты

| Компонент | Описание |
|-----------|----------|
| **Рефлективный загрузчик** | C-код с собственными UEFI-объявлениями и прямыми вызовами протоколов UEFI |
| **Потоковое преобразование байтов** | Собственное преобразование с обновлением внутреннего состояния после каждого байта |
| **RLE-сжатие** | Кодирование серий одинаковых байтов для уменьшения размера |
| **Проверки среды выполнения** | Опциональные проверки виртуализации и отладки |
| **Загрузка в памяти** | Полезная нагрузка распаковывается и отображается в памяти, а не сохраняется отдельным файлом |

## ✨ Возможности

| Возможность | Описание |
|-------------|----------|
| 🗜️ **Сжатие** | Собственное RLE-кодирование; степень сжатия зависит от входного файла |
| 🔐 **Преобразование данных** | Потоковое преобразование байтов с обратной связью между последовательными байтами |
| 🧠 **Рефлективная загрузка** | Отображение PE-образа, копирование секций, применение релокаций и обновление ImageBase |
| 🧹 **Удаление заголовков** | Опциональное стирание PE-заголовков отображённого образа после загрузки |
| 🛠️ **Две конфигурации сборки** | Clang + LLD или GCC + MinGW-w64 (w64devkit64) |
| 📦 **Компактный stub** | Текущий размер заглушки загрузчика — примерно **4 KB**; итог зависит от компилятора и параметров сборки |

## 🔒 Полный конвейер упаковки

```text
ЭТАП 1: ПРОВЕРКА PE
├── Проверка сигнатур MZ и PE
├── Проверка архитектуры x86-64
├── Проверка формата PE32+ и EFI Application subsystem
└── Проверка настроенных ограничений размера

ЭТАП 2: СЖАТИЕ
└── Собственное RLE-кодирование

ЭТАП 3: ПРЕОБРАЗОВАНИЕ ДАННЫХ
├── Генерация ключа размером 16–128 байт
└── Потоковое преобразование с обновлением состояния

ЭТАП 4: СБОРКА STUB
├── Использование предварительно собранного EFI PE32+ загрузчика
├── Добавление ключа и сжатой/преобразованной полезной нагрузки
├── Сохранение длины overlay
└── Формирование единого EFI-файла

ЭТАП 5: ВЫПОЛНЕНИЕ ЗАГРУЗЧИКА
├── Чтение overlay из собственного EFI-файла
├── Обратное преобразование и распаковка в памяти
├── Выделение страниц, копирование PE-секций и применение релокаций
├── Обновление информации загруженного образа
├── Применение опциональных проверок
├── Опциональное стирание PE-заголовков
└── Передача управления точке входа полезной нагрузки
```

## 🚀 Быстрый старт

### 📥 Получение исходного кода

```bash
git clone https://github.com/vk-candpython/efildr.git
cd efildr
```

### 🛠️ Сборка загрузчика

Загрузчик напрямую собирается в **UEFI x86-64 PE32+**. Доступны два альтернативных варианта:

- **Clang + LLD** — команда для POSIX shell приведена в разделе [English → Build the Loader](#build-the-loader).
- **GCC + MinGW-w64** — команда для Windows CMD через w64devkit64 приведена там же.

Обе команды создают `loader.efi`; запускать их последовательно не нужно.

### 🐍 Упаковщик Python

```bash
python3 efildr.py myapp.efi
```

Упаковщик сжимает и преобразует `myapp.efi`, после чего добавляет overlay к копии загрузчика и создаёт `./efildr-myapp.efi`.

### 🧪 Тестирование в QEMU

```bash
mkdir -p disk/EFI/BOOT
cp ./efildr-myapp.efi disk/EFI/BOOT/BOOTX64.EFI
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd \
    -drive format=raw,file=fat:rw:disk -net none -m 256M
```

## ⚙️ Флаги конфигурации времени сборки

В `loader.h` доступны три переключателя времени компиляции:

```c
#define USING_ANTI_VM          0
#define USING_ANTI_DEBUG       0
#define USING_ERASE_PE_HEADERS 0
```

| Флаг | Значение по умолчанию | Описание |
|------|-----------------------|----------|
| **`USING_ANTI_VM`** | `0` | Включает проверки виртуализированных окружений |
| **`USING_ANTI_DEBUG`** | `0` | Включает проверки на наличие отладчика |
| **`USING_ERASE_PE_HEADERS`** | `0` | Стирает DOS/NT-заголовки отображённого образа после загрузки |

Изменение любого флага требует пересборки `loader.efi` перед упаковкой payload.

## ⚙️ Технические особенности

- **Прямые вызовы протоколов EFI** — функции Boot Services и протоколов вызываются через собственный макрос `EFI_CALL` с автоматическим подсчётом аргументов.
- **Цепочки вызовов** — `IF_EFIFAIL_CHAINCALL` объединяет несколько вызовов и прекращает цепочку при ошибке EFI.
- **Потоковое преобразование** — `DEC_BYTE` выполняет собственное преобразование байта и обновляет внутреннее состояние для следующего байта. Алгоритм не заявляется как прошедший независимый криптографический аудит.
- **Рефлективная загрузка** — образ отображается, перемещается и запускается из памяти; сведения о загруженном образе обновляются.
- **Опциональные проверки** — переключатели времени компиляции управляют проверками среды и отладки.
- **Очистка памяти** — выделенные буферы и открытые файловые дескрипторы освобождаются на предусмотренных путях очистки.
- **Буфер EFI File Info** — размер буфера `EFI_FILE_INFO` вычисляется по пути к образу до выделения памяти.

## 📁 Результат

```text
original.efi  →  efildr-original.efi
```

- Один EFI-файл содержит загрузчик и упакованную полезную нагрузку.
- Полезная нагрузка не сохраняется отдельным файлом в открытом виде.
- Текущий размер stub загрузчика — примерно **4 KB**; итоговый упакованный файл будет больше и зависит от входного EFI.
- В приведённых Release-командах выполняется удаление символов (`-s`).

## ⚠️ Требования

| Требование | Версия / инструмент | Примечания |
|------------|--------------------|------------|
| **Компилятор — вариант A** | Clang + LLD | Target: `x86_64-w64-mingw32` |
| **Компилятор — вариант B** | GCC + MinGW-w64 | 64-битная сборка, например w64devkit64 |
| **Упаковщик** | Python 3.6+ | Дополнительные пакеты не требуются |
| **Целевая платформа** | UEFI x86-64 (PE32+) | EFI Application subsystem (`10`) |
| **Опциональное тестирование** | QEMU + OVMF | `qemu-system-x86_64` и образ прошивки OVMF |

Для двух приведённых способов прямой сборки PE/COFF не требуются заголовки GNU-EFI, `crt0-efi-x86_64.o` или преобразование ELF в PE.

## 🔧 Устранение неполадок

| Проблема | Решение |
|----------|---------|
| `clang: linker command failed` | Проверь, что LLD установлен и доступен Clang через `-fuse-ld=lld`. |
| GCC не распознаёт опцию линкера | Убедись, что используется MinGW-w64 GCC из w64devkit64 и PE-версия GNU `ld`. |
| Упаковщик сообщает `Invalid EFI image` | Входной файл должен быть 64-битным EFI-приложением (PE32+, subsystem `10`). Проверь его через `objdump -p`. |
| QEMU загружается в EFI Shell | Проверь расположение `disk/EFI/BOOT/BOOTX64.EFI` и путь к OVMF. |
| Ошибка `overlay corrupted` | Убедись, что версии загрузчика и упаковщика используют одинаковую структуру overlay и соглашения о размере ключа. |
| `file size too large` | По умолчанию максимальный размер входного файла упаковщика — 8 MiB; при необходимости измени настроенный лимит. |

---

<div align="center">

**[⬆ Back to Top](#-efildr)**

*Reflective EFI PE Loader & Packer*

</div>
