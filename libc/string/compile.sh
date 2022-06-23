#!/bin/bash

#allow for errors
set -e

export PREFIX="../../../opt/cross"
export PATH="$PREFIX/bin:$PATH"
echo "Compiling string libraray"
i686-elf-gcc -c *.c -ffreestanding -std=gnu99 -O2 -Wall -Wextra
echo "Done"
echo "Moving object files"
mv *.o ../obj
echo "Done, string library done"
