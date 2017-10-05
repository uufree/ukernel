#!/bin/bash
g++ -c -o main.o main.cc 
ld main.o -Ttext 0xc0001500 -e main -o kernel.bin
dd if=./kernel.bin of=../bochs/bin/ukernel.img bs=512 count=200 seek=9 conv=notrunc
