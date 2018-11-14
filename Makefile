#Entry point of QiuOS
ENTRYPOINT	=	0x30400

#Command 
ASM		=	nasm
LD		=	ld 
CC		=	gcc

#Parament of command
ASMBFLAGS	=	-I boot/include/
ASMKFLAGS	=   -f elf
CFLAGS		=	-c -I include/ -m32
LDFLAGS		=	-s -m elf_i386 -Ttext 0x30400

#This program
QiuOSBOOT	=	boot/boot.bin	boot/loader.bin
OBJS		=	lib/kliba.o 	kernel/kernel.o 	kernel/start.o 	
QiuOSKERNEL	=	kernel/kernel.bin

.PHONY:		everything clean all bulidimg

#Default start position
everything:	$(QiuOSBOOT) $(QiuOSKERNEL)

clean:		
			rm -f $(OBJS) $(QiuOSBOOT) $(QiuOSKERNEL)

all:		clean everything bulidimg



bulidimg:	
			dd if=boot/boot.bin of=boot/a.img bs=512 count=1 conv=notrunc
			sudo mount a.img /mnt 
			sudo cp boot/loader.bin /mnt
			sudo cp kernel/kernel.bin /mnt
			sudo umount /mnt

boot/boot.bin:boot/boot.asm boot/include/fat12hdr.inc boot/include/pm.inc
			$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin:boot/loader.asm boot/include/fat12hdr.inc boot/include/pm.inc
			$(ASM) $(ASMBFLAGS) -o $@ $<

lib/kliba.o:lib/kliba.asm
			$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/kernel.o:kernel/kernel.asm 
			$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/start.o:kernel/start.c include/const.h include/type.h include/protect.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/kernel.bin:lib/kliba.o kernel/kernel.o kernel/start.o 	
#			ld -s -m elf_i386 -Ttext 0x30400 -o kernel/kernel.bin lib/kliba.o kernel/kernel.o kernel/start.o 	
			ld -s -m elf_i386 -Ttext 0x30400 -o kernel/kernel.bin kernel/kernel.o kernel/start.o lib/kliba.o