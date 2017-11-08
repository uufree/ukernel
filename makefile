###########################SET COMPLIER#########################
ENTRY_POINT = 0xc0001500
AS = nasm
CXX = g++
LD = ld

##########################SET COMPLIER FLAGS############################
ASFLAGS = -f elf
CXXFLAGS = -m32 -c -Wall -fno-stack-protector
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main 

OBJS = ./build/main.o ./build/init.o ./build/interrupt.o ./build/print.o ./build/kernel.o ./build/debug.o ./build/timer.o

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

./build/main.o: main.cc 
	$(CXX) $(CXXFLAGS) -o $@ $^

./build/init.o: init.cc 
	$(CXX) $(CXXFLAGS) -o $@ $^

./build/interrupt.o:interrupt.cc 
	$(CXX) $(CXXFLAGS) -o $@ $^

./build/timer.o: timer.cc  
	$(CXX) $(CXXFLAGS) -o $@ $^

./build/debug.o: debug.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

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
	
