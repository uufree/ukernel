########################SET COMPLIER##########################
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld

########################SET COMPLIER FLAGS###################
ASFLAGS = -f elf
CFLAGS = -m32 -Wall $(LIB) -c -fno-stack-protector 
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main 

BUILD_DIR = ./build
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
      $(BUILD_DIR)/timer.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o \
      $(BUILD_DIR)/debug.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/bitmap.o \
      $(BUILD_DIR)/string.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/list.o \
      $(BUILD_DIR)/switch.o $(BUILD_DIR)/console.o $(BUILD_DIR)/sync.o \
      $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/tss.o \
      $(BUILD_DIR)/process.o $(BUILD_DIR)/syscall.o $(BUILD_DIR)/syscall-init.o \
      $(BUILD_DIR)/stdio.o $(BUILD_DIR)/ide.o $(BUILD_DIR)/fs.o \
	  $(BUILD_DIR)/inode.o $(BUILD_DIR)/file.o $(BUILD_DIR)/dir.o $(BUILD_DIR)/fork.o \
	  $(BUILD_DIR)/shell.o $(BUILD_DIR)/assert.o $(BUILD_DIR)/buildin_cmd.o \
	  $(BUILD_DIR)/exec.o $(BUILD_DIR)/wait_exit.o $(BUILD_DIR)/pipe.o

##############     c代码编译     ###############
$(BUILD_DIR)/main.o: main.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: init.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: interrupt.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: timer.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: debug.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: string.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: bitmap.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: memory.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: thread.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/list.o: list.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/console.o: console.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: sync.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: keyboard.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ioqueue.o: ioqueue.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/tss.o: tss.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/process.o: process.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall.o: syscall.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall-init.o: syscall-init.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio.o: stdio.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ide.o: ide.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fs.o: fs.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/inode.o: inode.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/file.o: file.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/dir.o: dir.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fork.o: fork.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/shell.o: shell.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/assert.o: assert.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/buildin_cmd.o: buildin_cmd.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/exec.o: exec.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/wait_exit.o: wait_exit.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/pipe.o: pipe.c 
	$(CC) $(CFLAGS) $< -o $@


##############    汇编代码编译    ###############
$(BUILD_DIR)/kernel.o: kernel.asm
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/print.o: print.asm
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/switch.o: switch.asm
	$(AS) $(ASFLAGS) $< -o $@

##############    链接所有目标文件    #############
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY : mk_dir hd clean all

mk_dir:
	if [[ ! -d $(BUILD_DIR) ]];then mkdir $(BUILD_DIR);fi

hd:
	dd if=$(BUILD_DIR)/kernel.bin \
           of=/home/uuchen/bochs/bin/ukernel.img \
           bs=512 count=200 seek=9 conv=notrunc

clean:
	cd $(BUILD_DIR) && rm -f ./*

build: $(BUILD_DIR)/kernel.bin

all: mk_dir build hd
