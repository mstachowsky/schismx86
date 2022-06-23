#!/bin/bash

#set it so that a failed command exits the script
set -e

export PREFIX="../../opt/cross"
export PATH="$PREFIX/bin:$PATH"

echo "Cleaning directory"
cd obj

if test -f '*.o'; then
	rm *.o
fi

cd ..

echo "Compiling stdio"
i686-elf-gcc -c stdio.c -o stdio.o -std=gnu99 -ffreestanding -fpic -O2 -Wall -Wextra
echo "Done"
echo "Compiling syscall"
i686-elf-gcc -c syscall.c -o syscall.o -std=gnu99 -ffreestanding -fpic -O2 -Wall -Wextra
echo "Done"
echo "Compiling crt0"
i686-elf-gcc -c crt0.c -o crt0.o -std=gnu99 -ffreestanding -fpic -O2 -Wall -Wextra
echo "Done"

echo "Compiling all directories"
	for dir in `ls .`;
		do
			test -d "$dir" || continue
			test -f $dir/compile.sh || continue
			cd $dir
			bash compile.sh
			cd ..
		done
echo "Done"

echo "Moving object files"
mv *.o obj
echo "Done"

echo "Archiving into libc.a"
ar -cvq libc.a obj/*.o
echo "Done"

echo "+++++++++++++"
echo "=============================="
echo "Done! libc is ready to go"
echo "=============================="
echo "+++++++++++++"
