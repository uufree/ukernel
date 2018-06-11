###########################SET COMPLIER#########################
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld

##########################SET COMPLIER FLAGS############################
ASFLAGS = -f elf
CFLAGS = -m32 -c -Wall -fno-stack-protector
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main 

OBJS = ./build/main.o ./build/bitmap.o ./build/console.o ./build/debug.o ./build/init.o ./build/interrupt.o ./build/ioqueue.o ./build/kernel.o ./build/keyboard.o ./build/list.o ./build/memory.o ./build/print.o ./build/process.o ./build/stdio.o ./build/string.o ./build/switch.o ./build/sync.o ./build/syscall.o ./build/syscall-init.o ./build/thread.o ./build/timer.o ./build/tss.o ./build/ide.o

##########################SET C LANGUARE##############################
kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

./build/main.o: main.c 
	$(CC) $(CFLAGS) -o $@ $^

./build/bitmap.o: bitmap.c 
	$(CC) $(CFLAGS) -o $@ $^

./build/console.o: console.c 
	$(CC) $(CFLAGS) -o $@ $^

./build/debug.o: debug.c 
	$(CC) $(CFLAGS) -o $@ $^

./build/init.o: init.c
	$(CC) $(CFLAGS) -o $@ $^

./build/interrupt.o: interrupt.c
	$(CC) $(CFLAGS) -o $@ $^

./build/ioqueue.o: ioqueue.c
	$(CC) $(CFLAGS) -o $@ $^

./build/keyboard.o: keyboard.c
	$(CC) $(CFLAGS) -o $@ $^

./build/list.o: list.c
	$(CC) $(CFLAGS) -o $@ $^

./build/memory.o: memory.c
	$(CC) $(CFLAGS) -o $@ $^

./build/process.o: process.c
	$(CC) $(CFLAGS) -o $@ $^

./build/stdio.o: stdio.c
	$(CC) $(CFLAGS) -o $@ $^

./build/string.o: string.c
	$(CC) $(CFLAGS) -o $@ $^

./build/sync.o: sync.c
	$(CC) $(CFLAGS) -o $@ $^

./build/syscall.o: syscall.c
	$(CC) $(CFLAGS) -o $@ $^

./build/syscall-init.o: syscall-init.c
	$(CC) $(CFLAGS) -o $@ $^

./build/thread.o: thread.c
	$(CC) $(CFLAGS) -o $@ $^

./build/timer.o: timer.c
	$(CC) $(CFLAGS) -o $@ $^

./build/tss.o: tss.c
	$(CC) $(CFLAGS) -o $@ $^

./build/ide.o: ide.c
	$(CC) $(CFLAGS) -o $@ $^

###########################SET ASSEMBLY LANGUAGE#################
./build/kernel.o: kernel.asm
	$(AS) $(ASFLAGS) -o $@ $^

./build/print.o: print.asm
	$(AS) $(ASFLAGS) -o $@ $^

./build/switch.o: switch.asm
	$(AS) $(ASFLAGS) -o $@ $^

###########################SET .PHONY########################
.PHONY: all clean

all:
	dd if=./kernel.bin of=/home/uuchen/bochs/bin/ukernel.img bs=512 count=200 seek=9 conv=notrunc
clean:
	rm -rf ./build/*.o
	rm kernel.bin	
