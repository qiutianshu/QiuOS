


.PHONY: all

all:echo.o ps.o bxtar.o pi.o d1 d2 d3 d4 tar dd

clean:		
	 rm -f echo.o ps.o bxtar.o pi.o

echo.o:echo.c
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o echo.o echo.c

ps.o:ps.c
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o ps.o ps.c

bxtar.o:bxtar.c
	  gcc -I ../include/ -m32 -c -fno-builtin -Wall -o bxtar.o bxtar.c

pi.o:pi.c
	gcc -I ../include/ -m32 -c -fno-builtin -Wall -o pi.o pi.c

d1:echo.o start.o
	ld -Ttext 0x1000 -m elf_i386 -o echo echo.o start.o ../lib/crt.a 

d2:ps.o start.o
	ld -Ttext 0x1000 -m elf_i386 -o ps ps.o start.o ../lib/crt.a 

d3:bxtar.o start.o
	ld -Ttext 0x1000 -m elf_i386 -o bxtar bxtar.o start.o ../lib/crt.a 

d4:pi.o start.o
	ld -Ttext 0x1000 -m elf_i386 -o pi pi.o start.o ../lib/crt.a 

tar:ps bxtar echo pi kernel.bin loader.bin
	tar vcf cmd.tar loader.bin kernel.bin ps bxtar echo pi

dd:
	dd if=cmd.tar of=../80m.img seek=27131392 bs=1 count=102400 conv=notrunc
