###########################SET COMPLIER#########################
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld

##########################SET COMPLIER FLAGS############################
ASFLAGS = -f elf
CFLAGS = -m32 -c -Wall -fno-stack-protector
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main 

OBJS = ./build/main.o ./build/init.o ./build/interrupt.o ./build/print.o ./build/kernel.o ./build/debug.o ./build/timer.o

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

./build/main.o: main.c 
	$(CC) $(CFLAGS) -o $@ $^

./build/init.o: init.c 
	$(CC) $(CFLAGS) -o $@ $^

./build/interrupt.o: interrupt.c 
	$(CC) $(CFLAGS) -o $@ $^

./build/timer.o: timer.c
	$(CC) $(CFLAGS) -o $@ $^

./build/debug.o: debug.c
	$(CC) $(CFLAGS) -o $@ $^

./build/print.o: print.asm
	$(AS) $(ASFLAGS) -o $@ $^

./build/kernel.o: kernel.asm
	$(AS) $(ASFLAGS) -o $@ $^

###########################SET .PHONY########################
.PHONY: all clean

all:
	dd if=./kernel.bin of=../bochs/bin/ukernel.img bs=512 count=200 seek=9 conv=notrunc
clean:
	rm -rf ./build/*.o
	rm ukernel.bin	
