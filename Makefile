CCFLAGS=-c -I./src/include -ffreestanding -O2 -m32 -mno-sse
LDFLAGS=-T./src/lxboot.ld -nostdlib -m elf_i386
CC=x86_64-lux-gcc
LD=x86_64-lux-ld
SRC:=$(shell find ./src/core -type f -name "*.c")
OBJ:=$(SRC:.c=.o)

all: mbr.bin bootsec.bin lxboot.core lxboot.bin

mbr.bin: src/bootsect/mbr.asm
	@echo "\x1B[0;1;32m [  AS   ]\x1B[0m src/bootsect/mbr.asm"
	@nasm -f bin src/bootsect/mbr.asm -o mbr.bin

bootsec.bin: src/bootsect/bootsec.asm
	@echo "\x1B[0;1;32m [  AS   ]\x1B[0m src/bootsect/bootsec.asm"
	@nasm -f bin src/bootsect/bootsec.asm -o bootsec.bin

%.o: %.c
	@echo "\x1B[0;1;32m [  CC   ]\x1B[0m $<"
	@$(CC) $(CCFLAGS) -o $@ $<

lxboot.core: $(OBJ) src/core/stub.asm
	@echo "\x1B[0;1;32m [  AS   ]\x1B[0m src/core/stub.asm"
	@nasm -f elf src/core/stub.asm -o src/core/stub.o
	@echo "\x1B[0;1;34m [  LD   ]\x1B[0m lxboot.core"
	@$(LD) $(LDFLAGS) src/core/stub.o $(OBJ) -o lxboot.core

lxboot.bin: src/*.asm lxboot.core
	@echo "\x1B[0;1;32m [  AS   ]\x1B[0m src/main.asm"
	@nasm -f bin src/main.asm -o lxboot.bin

clean:
	@rm -f mbr.bin bootsec.bin lxboot.bin $(OBJ)
