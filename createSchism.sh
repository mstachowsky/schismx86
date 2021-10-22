#!/bin/bash

export PREFIX="../opt/cross"
export PATH="$PREFIX/bin:$PATH"

echo "Cleaning directory"
rm *.o
rm schism.iso
echo "Done"
echo "Assembling boot.s"
i686-elf-as  boot.s -o boot.o
echo "Done"
echo "Compiling kernel.c"
i686-elf-gcc -Iinclude -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling IOPort Library"
i686-elf-gcc -Iinclude -c include/schismIOPort.c -o schismIOPort.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling PCI Library"
i686-elf-gcc -Iinclude -c include/schismPCI.c -o schismPCI.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling Kernel IO Library"
i686-elf-gcc -Iinclude -c include/schismKernelIO.c -o schismKernelIO.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling PS/2 Library"
i686-elf-gcc -Iinclude -c include/schismPS2.c -o schismPS2.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Linking kernel"
i686-elf-gcc -T linker.ld -o schism.bin -ffreestanding -O2 -nostdlib *.o -lgcc
echo "Done"
echo "Moving bin files"
cp schism.bin iso/boot/schism.bin
echo "Done"
echo "Creating bootable iso"
sudo grub-mkrescue -o schism.iso iso
echo "Done"
echo "Copying to host"
cp schism.iso /mnt/c/Users/tienb
echo "Done! Schism is ready to go"
