
all: mbr.bin bootsec.bin

mbr.bin: src/bootsect/mbr.asm
	@echo "\x1B[0;1;32m [  AS   ]\x1B[0m src/bootsect/mbr.asm"
	@nasm -f bin src/bootsect/mbr.asm -o mbr.bin

bootsec.bin: src/bootsect/bootsec.asm
	@echo "\x1B[0;1;32m [  AS   ]\x1B[0m src/bootsect/bootsec.asm"
	@nasm -f bin src/bootsect/bootsec.asm -o bootsec.bin

clean:
	@rm -rf mbr.bin bootsec.bin
