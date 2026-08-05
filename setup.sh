#!/bin/bash
set -e


echo "==================================================="
echo "(C) Vladislav Khudash, 2026"
echo "(P) GitHub: https://github.com/vk-candpython/efildr"
echo ""


#
# Author   : Vladislav Khudash
# Source   : https://github.com/vk-candpython/efildr/blob/main/setup.sh
# Compiler : GCC (GNU-EFI)
# Platform : LINUX x86_64
# Summary  : Builds an optimized PE32+ UEFI loader.
#


ARCH=x86_64


SRC_C=loader.c
SRC_O=loader.o
SRC_SO=loader.so
SRC_EFI=loader.efi


EFI_INC=/usr/include/efi
EFI_LIBS_DIR="/usr/lib"

EFI_CRT=${EFI_LIBS_DIR}/crt0-efi-${ARCH}.o
EFI_LDS=${EFI_LIBS_DIR}/elf_${ARCH}_efi.lds



gcc -c ${SRC_C} -o ${SRC_O}                                               \
    -I${EFI_INC} -I${EFI_INC}/${ARCH} -I${EFI_INC}/protocol               \
    -Wall -Wextra                                                         \
    -m64 -O3 -fpic -g0                                                    \
    -nostdlib -ffreestanding -fshort-wchar -mno-red-zone                  \
    -fno-semantic-interposition -fipa-pta -fstrict-aliasing               \
    -fvisibility=hidden -fomit-frame-pointer                              \
    -fmerge-all-constants -ffunction-sections -fdata-sections             \
    -fno-stack-check -fno-stack-protector -fno-stack-clash-protection     \
    -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-exceptions    \
    -fno-ident -fno-common -fno-plt

echo "1. [+] Compiled  : ${SRC_C} -> ${SRC_O}"


ld ${SRC_O} -o ${SRC_SO}                                    \
   -T ${EFI_LDS} ${EFI_CRT}                                 \
   ${EFI_LIBS_DIR}/libgnuefi.a ${EFI_LIBS_DIR}/libefi.a     \
   --no-undefined -nostdlib -shared -Bsymbolic              \
   --as-needed --sort-common --build-id=none --strip-all    \
   -Os

echo "2. [+] Linked    : ${SRC_O} -> ${SRC_SO}"


objcopy ${SRC_SO} ${SRC_EFI}           \
        -j .text -j .data -j .reloc    \
        --target=efi-app-${ARCH}       \
        --strip-all

echo "3. [+] Converted : ${SRC_SO} -> ${SRC_EFI} (PE32+)"


rm -f ${SRC_O} ${SRC_SO}

echo "4. [+] Cleaned   : ${SRC_O}, ${SRC_SO}"



echo "5. [+] Built     : ${SRC_EFI} ($(stat -c %s ${SRC_EFI}) bytes)"
echo "==================================================="
