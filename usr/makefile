


.PHONY: all

all:start d tar dd

start:
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o echo.o echo.c

d:
	ld -Ttext 0x1000 -m elf_i386 -o echo echo.o start.o ../lib/crt.a 

tar:
	tar vcf cmd.tar echo start.o

dd:
	dd if=cmd.tar of=../80m.img seek=27131392 bs=1 count=102400 conv=notrunc
