#Entry point of QiuOS
ENTRYPOINT	=	0x30400

#Command 
ASM		=	nasm
LD		=	ld 
CC		=	gcc

#Parament of command
ASMBFLAGS	=	-I boot/include/
ASMKFLAGS	=   -f elf -I include/
CFLAGS		=	-c -I include/ -m32
LDFLAGS		=	-s -m elf_i386 -Ttext 0x30400

#This program
QiuOSBOOT	=	boot/boot.bin	boot/loader.bin
OBJS		=	lib/kliba.o 	kernel/kernel.o kernel/start.o 	kernel/i8259.o kernel/protect.o lib/klib.o kernel/main.o kernel/global.o kernel/clock.o kernel/syscall.o kernel/proc.o
QiuOSKERNEL	=	kernel/kernel.bin

.PHONY:		everything clean all bulidimg

#Default start position
everything:	$(QiuOSBOOT) $(OBJS) $(QiuOSKERNEL)

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

kernel/syscall.o:kernel/syscall.asm
			$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/proc.o:kernel/proc.c include/type.h include/const.h include/global.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o:kernel/protect.c include/const.h include/type.h include/global.h include/proto.h include/protect.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/start.o:kernel/start.c include/const.h include/type.h include/protect.h include/proto.h include/global.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/main.o:kernel/main.c include/const.h include/type.h include/protect.h include/proto.h include/global.h include/proc.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/global.o:kernel/global.c include/const.h include/type.h include/protect.h include/global.h include/proc.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o:kernel/i8259.c include/const.h include/type.h include/protect.h include/proto.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o:kernel/clock.c include/const.h include/type.h include/protect.h include/proto.h include/proc.h include/global.h
			$(CC) $(CFLAGS) -o $@ $<

lib/klib.o:lib/klib.c include/proto.h include/const.h include/type.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/kernel.bin:lib/klib.o lib/kliba.o kernel/kernel.o kernel/start.o kernel/protect.o kernel/i8259.o kernel/main.o kernel/global.o kernel/clock.o kernel/proc.o kernel/syscall.o
			ld -s -m elf_i386 -Ttext 0x30400 -o kernel/kernel.bin kernel/kernel.o kernel/main.o kernel/clock.o kernel/proc.o kernel/syscall.o kernel/start.o kernel/protect.o kernel/i8259.o lib/klib.o lib/kliba.o kernel/global.o