


.PHONY: all

all:start1 start2 d1 d2 tar dd

start1:
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o echo.o echo.c

start2:
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o ps.o ps.c

d1:
	ld -Ttext 0x1000 -m elf_i386 -o echo echo.o start.o ../lib/crt.a 

d2:
	ld -Ttext 0x1000 -m elf_i386 -o ps ps.o start.o ../lib/crt.a 

tar:
	tar vcf cmd.tar echo  ps

dd:
	dd if=cmd.tar of=../80m.img seek=27131392 bs=1 count=102400 conv=notrunc
