# 👾 efildr


<div align="center">

[![Platform](https://img.shields.io/badge/platform-UEFI-blue?logo=uefi&logoColor=white)](https://uefi.org)
[![Language](https://img.shields.io/badge/language-C%2FC%2B%2B-00599C?logo=c%2B%2B)](https://en.cppreference.com/)
[![Builder](https://img.shields.io/badge/builder-Python%203-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

*Compress, Encrypt, Fileless Execute — One EFI binary, zero traces.*

</div>

---

> [!WARNING]
> This tool is intended **only** for educational purposes and authorized security auditing.  
> The author assumes no liability for any misuse or damage.

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

**EFILDR** — is a reflective EFI loader and packer that transforms standard UEFI applications (PE32+) into self‑contained, encrypted, fileless‑executing EFI binaries.  
It combines efficient RLE compression, a unique ARX stream cipher, and a fully reflective loader stub written in pure C — no CRT, no imports, all EFI protocols called directly.

The loader never writes the original image to disk: it reads its own overlay, decrypts and decompresses the payload directly in memory, maps it reflectively, and transfers execution. The result is a single EFI executable that hides the original code and evades common static and dynamic analysis.

### Key Components

| Component | Description |
|-----------|-------------|
| **Reflective Loader** | Pure C, no standard libraries; all EFI protocols used via custom call‑gate macros |
| **ARX Stream Cipher** | Unique stateful byte mixer with data‑dependent rotation and non‑linear key scheduling |
| **RLE Compression** | Efficient run‑length encoding reduces payload size |
| **Runtime Protection** | Optional anti‑analysis and environmental checks |
| **Fileless Execution** | Everything happens in memory — no disk writes of the original EFI |

## ✨ Features

### Core Packing

| Feature | Description |
|---------|-------------|
| 🗜️ **Compression** | Custom RLE reduces size by 10‑40% on typical EFI files |
| 🔐 **Encryption** | ARX stream cipher with feedback — each byte depends on all previous |
| 🧠 **Reflective Loading** | Full PE mapping: sections, relocations, image base update |
| 🧹 **Header Erasure** | Optional removal of PE headers from memory after load |

### Compilation & Linking

| Feature | Description |
|---------|-------------|
| 🏗️ **Minimal Environment** | `-nostdlib -ffreestanding` — no CRT |
| 🧹 **Stripped Binary** | `--strip-all`, no symbols, minimal footprint |
| ⚡ **Optimised Output** | LTO, `-O3`, section garbage collection, tiny 36 KB loader |

### Runtime Protection

| Feature | Description |
|---------|-------------|
| 🛡️ **Anti‑analysis** | Detects debugging and virtualised environments |
| 🔒 **Memory Cleanup** | All allocated buffers are properly freed and closed before exit |

### Builder (Python)

| Feature | Description |
|---------|-------------|
| 🐍 **Cross‑platform** | Pure Python 3, no external dependencies |
| 🔑 **Variable Key Length** | Supports 16, 32, 64, or 128‑byte keys |
| 📊 **Statistics** | Displays original size, compressed size, saving percentage, and elapsed time |

## 🔒 Complete Packing Pipeline

```
PHASE 1: PE VALIDATION
├── Checks signatures (MZ, PE)
├── Confirms x86‑64 machine type
├── Verifies PE32+ magic and EFI application subsystem
└── Enforces size limits (512 B – 8 MiB)

PHASE 2: COMPRESSION
├── Custom run‑length encoding (min run 3, max 127)
└── Typical size reduction: 10‑40%

PHASE 3: ENCRYPTION
├── Random key generation (16‑128 bytes)
├── ARX stream cipher with internal state
└── Feedback makes decryption dependent on all previous bytes

PHASE 4: STUB ASSEMBLY
├── Loader stub (pre‑compiled EFI PE32+)
├── Key, compressed + encrypted payload appended
├── Overlay length stored for runtime reading
└── Single .efi file ready to deploy

PHASE 5: RUNTIME (Loader Execution)
├── Reads overlay from its own EFI file
├── Decrypts and decompresses payload in memory
├── Allocates pages, copies sections, applies relocations
├── Updates loaded image protocol with new base and size
├── Applies runtime protection (if enabled)
├── Optionally erases PE headers from memory
└── Jumps to the entry point (or returns error with reboot)
```

## 🚀 Quick Start

### 📥 Build the Loader

The loader is written in C and compiled with GCC (GNU‑EFI). A helper script `setup.sh` automates the whole process:

```bash
git clone https://github.com/vk-candpython/efildr.git
cd efildr
chmod +x setup.sh
./setup.sh
```

This produces `loader.efi` — the reflective loader stub.

### 🐍 Builder (Python)

```bash
python3 efildr.py myapp.efi
```

The builder compresses and encrypts `myapp.efi`, then attaches the overlay to a copy of the loader stub, creating `./efildr-myapp.efi`.

**Output example:**
```
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

The loader (`loader.h`) exposes three compile‑time switches that control its behaviour:

```c
#define USING_ANTI_VM          1
#define USING_ANTI_DEBUG       1
#define USING_ERASE_PE_HEADERS 1
```

| Flag | Default | Description |
|------|---------|-------------|
| **`USING_ANTI_VM`** | `1` | Enables detection of virtualised environments |
| **`USING_ANTI_DEBUG`** | `1` | Enables anti‑debugging checks |
| **`USING_ERASE_PE_HEADERS`** | `1` | Erases DOS/NT headers from memory after loading |

> Changing any flag requires rebuilding the loader stub (`./setup.sh`) before packing.

## ⚙️ Technical Highlights

- **Direct EFI Protocol Calls** – all Boot Services and protocol functions are called via a custom call‑gate macro (`EFI_CALL`), with automatic argument counting.
- **Short‑Circuit Call Chains** – `IF_EFIFAIL_CHAINCALL` allows chaining several protocol calls with proper error handling and cleanup, without nested `if`s.
- **ARX Stream Cipher** – the `DEC_BYTE` macro implements a multi‑round ARX transformation with data‑dependent rotation and prime‑based key scheduling, ensuring strong diffusion and uniqueness.
- **Reflective Loading** – the payload is mapped, relocated, and executed entirely from memory, using `EFI_LOADED_IMAGE_PROTOCOL` to update the loaded image information.
- **Anti‑analysis** – optional measures help to detect debugging and common analysis environments, without relying on easily hooked API calls.
- **Memory Cleanup** – all allocated buffers are properly freed and closed before exit, leaving no leaked resources.
- **Stack‑based File Info** – the overlay reader uses a compile‑time calculated buffer size for `EFI_FILE_INFO`, avoiding dynamic allocations and improving compatibility with OVMF.

## 📁 Output

```
original.efi  →  efildr-original.efi
```

- Single EFI executable containing loader + encrypted payload
- No imports, symbols, or plaintext strings
- Executes completely in memory
- Typical size of the loader stub: ~36 KB

## ⚠️ Requirements

| Requirement | Version | Notes |
|-------------|---------|-------|
| **Loader Compiler** | GCC (with GNU‑EFI headers) | `gcc`, `ld`, `objcopy` |
| **Builder** | Python 3.6+ | No extra packages needed |
| **Target Platform** | UEFI x86_64 (PE32+) | Tested with OVMF (QEMU) and real hardware |
| **Optional Testing** | QEMU + OVMF | `qemu-system-x86_64`, `ovmf` |

## 🔧 Troubleshooting

| Issue | Solution |
|-------|----------|
| `compilation fails` | Ensure GNU‑EFI development files are installed (`/usr/include/efi`, `crt0-efi-x86_64.o`, `elf_x86_64_efi.lds`). |
| `builder reports "Invalid EFI image"` | The input must be a 64‑bit EFI application (PE32+, subsystem 10). Check with `objdump -p`. |
| `QEMU boots to shell` | Place the .efi file as `disk/EFI/BOOT/BOOTX64.EFI`. QEMU will auto‑detect it. |
| `overlay corrupted` error in loader | Ensure you use the same builder version as the loader. Key size and layout must match. |
| `file size too large` | Maximum input size is 8 MiB by default. Adjust `EFI_FILE_MAX_SIZE` in the builder if needed. |

---

# Русский

## 📋 Обзор

**EFILDR** — это рефлективный EFI‑загрузчик и упаковщик, превращающий стандартные UEFI‑приложения (PE32+) в автономные, зашифрованные, бесфайлово‑исполняемые EFI‑бинарники.  
В основе лежат эффективное RLE‑сжатие, уникальный потоковый ARX‑шифр и загрузчик на чистом C, не требующий CRT и напрямую работающий с протоколами UEFI.

Загрузчик никогда не сохраняет исходный образ на диск: он читает собственный оверлей, расшифровывает и распаковывает полезную нагрузку прямо в памяти, отображает её рефлективно и передаёт управление. Результат — один EFI‑файл, скрывающий оригинальный код.

### Ключевые компоненты

| Компонент | Описание |
|-----------|----------|
| **Рефлективный загрузчик** | Чистый C, без CRT; все протоколы EFI вызываются через собственные макросы |
| **ARX‑шифр** | Уникальный потоковый шифр с обратной связью, зависящей от данных ротацией и нелинейным планированием ключей |
| **RLE‑сжатие** | Эффективное кодирование длин серий |
| **Защита времени выполнения** | Опциональные проверки окружения и отладки |
| **Бесфайловое выполнение** | Всё происходит в памяти — никакой записи оригинального EFI на диск |

## ✨ Возможности

*(Полный список см. в английской версии)*

## 🔒 Полный конвейер упаковки

*(Идентичен английской версии)*

## 🚀 Быстрый старт

```bash
git clone https://github.com/vk-candpython/efildr.git
cd efildr
chmod +x setup.sh
./setup.sh                                   # сборка загрузчика
python3 efildr.py myapp.efi                  # упаковка payload
```

При необходимости протестировать в QEMU:
```bash
mkdir -p disk/EFI/BOOT
cp ./efildr-myapp.efi disk/EFI/BOOT/BOOTX64.EFI
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd \
    -drive format=raw,file=fat:rw:disk -net none -m 256M
```

## ⚙️ Флаги конфигурации времени сборки

```c
#define USING_ANTI_VM          1
#define USING_ANTI_DEBUG       1
#define USING_ERASE_PE_HEADERS 1
```

| Флаг | Описание |
|------|----------|
| **`USING_ANTI_VM`** | Включает детект виртуальных машин |
| **`USING_ANTI_DEBUG`** | Включает защиту от отладки |
| **`USING_ERASE_PE_HEADERS`** | Затирает PE‑заголовки образа в памяти после загрузки |

## ⚙️ Технические особенности

- **Прямые вызовы протоколов EFI** – все функции Boot Services и протоколов вызываются через собственный макрос `EFI_CALL` с автоматическим подсчётом аргументов.
- **Цепочки вызовов** – `IF_EFIFAIL_CHAINCALL` позволяет объединять несколько вызовов протоколов с корректной обработкой ошибок и освобождением ресурсов.
- **ARX‑шифр** – макрос `DEC_BYTE` реализует многопроходное ARX‑преобразование с зависящей от данных ротацией и нелинейным планированием ключей, обеспечивая сильную диффузию и уникальность.
- **Рефлективная загрузка** – образ отображается, перемещается и запускается целиком в памяти с обновлением `EFI_LOADED_IMAGE_PROTOCOL`.
- **Анти‑анализ** – опциональные меры помогают обнаружить отладку и виртуализацию без использования легко перехватываемых API‑вызовов.
- **Очистка памяти** – все выделенные буферы корректно освобождаются, а файловые дескрипторы закрываются перед выходом.
- **Стековый буфер File Info** – читатель оверлея использует вычисляемый на этапе компиляции размер буфера для `EFI_FILE_INFO`, избегая динамических выделений и улучшая совместимость с OVMF.

## 📁 Результат

```
оригинал.efi  →  efildr-оригинал.efi
```

- Один EFI‑файл, содержащий загрузчик + зашифрованную полезную нагрузку
- Без импортов, символов и строк в открытом виде
- Полностью выполняется в памяти
- Типичный размер заглушки загрузчика: ~36 КБ

## ⚠️ Требования

| Требование | Версия | Примечания |
|------------|--------|------------|
| **Компилятор загрузчика** | GCC (с заголовками GNU‑EFI) | `gcc`, `ld`, `objcopy` |
| **Билдер** | Python 3.6+ | Без дополнительных пакетов |
| **Целевая платформа** | UEFI x86_64 (PE32+) | Проверено с OVMF (QEMU) и реальным оборудованием |
| **Опциональное тестирование** | QEMU + OVMF | `qemu-system-x86_64`, `ovmf` |

## 🔧 Устранение неполадок

*(См. английскую секцию Troubleshooting)*

---

<div align="center">

**[⬆ Back to Top](#-efildr)**

*Reflective EFI PE Loader & Packer*

</div>
