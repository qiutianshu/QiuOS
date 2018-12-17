


.PHONY: all

all:echo ps bxtar d1 d2 d3 tar dd

echo:
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o echo.o echo.c

ps:
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o ps.o ps.c

bxtar:
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o bxtar.o bxtar.c

d1:
	ld -Ttext 0x1000 -m elf_i386 -o echo echo.o start.o ../lib/crt.a 

d2:
	ld -Ttext 0x1000 -m elf_i386 -o ps ps.o start.o ../lib/crt.a 

d3:
	ld -Ttext 0x1000 -m elf_i386 -o bxtar bxtar.o start.o ../lib/crt.a 

tar:
	tar vcf cmd.tar ps bxtar

dd:
	dd if=cmd.tar of=../80m.img seek=27131392 bs=1 count=102400 conv=notrunc
