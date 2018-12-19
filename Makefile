#Entry point of QiuOS
ENTRYPOINT	=	0x30400

#Command 
ASM		=	nasm
LD		=	ld 
CC		=	gcc
BO 		=   bochs

#Parament of command
ASMBFLAGS	=	-I boot/include/
ASMKFLAGS	=   -f elf -I include/
CFLAGS		=	-c -I include/ -m32
LDFLAGS		=   -m elf_i386 -Ttext 0x30400

#This program
QiuOSBOOT	=	boot/boot.bin	boot/loader.bin
OBJS		=	lib/kliba.o kernel/kernel.o kernel/start.o 	kernel/i8259.o kernel/protect.o \
				lib/klib.o kernel/main.o kernel/global.o kernel/clock.o kernel/syscall.o kernel/proc.o \
				kernel/keyboard.o kernel/tty.o kernel/console.o kernel/printf.o lib/misc.o kernel/systask.o \
				kernel/hd.o fs/fs.o fs/open.o lib/usr/open.o lib/usr/rdwt.o fs/read_write.o lib/usr/unlink.o \
				fs/unlink.o lib/string.o mm/mm.o mm/forkexit.o lib/usr/fork.o lib/usr/exit.o lib/usr/wait.o \
				mm/exit.o lib/usr/getpid.o lib/cstring.o lib/usr/exec.o mm/exec.o lib/usr/stat.o fs/stat.o \
				kernel/shell.o

QiuOSKERNEL	=	kernel/kernel.bin

.PHONY:		everything clean all

#Default start position
everything:	$(QiuOSBOOT) $(OBJS) $(QiuOSKERNEL)

clean:		
			rm -f $(OBJS) $(QiuOSBOOT) $(QiuOSKERNEL)

all:		clean everything start

boot/boot.bin:boot/boot.asm boot/include/fat12hdr.inc boot/include/pm.inc
			$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin:boot/loader.asm boot/include/fat12hdr.inc boot/include/pm.inc
			$(ASM) $(ASMBFLAGS) -o $@ $<

lib/kliba.o:lib/kliba.asm
			$(ASM) $(ASMKFLAGS) -o $@ $<

lib/cstring.o:lib/cstring.asm
			$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/kernel.o:kernel/kernel.asm 
			$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/syscall.o:kernel/syscall.asm
			$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/systask.o:kernel/systask.c include/const.h include/type.h include/protect.h include/proto.h include/global.h include/proc.h include/tty.h include/console.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/proc.o:kernel/proc.c include/type.h include/const.h include/global.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o:kernel/printf.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/console.o:kernel/console.c include/const.h include/type.h include/protect.h include/proto.h include/global.h include/proc.h include/tty.h include/console.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/keyboard.o:kernel/keyboard.c include/const.h include/type.h include/protect.h include/proto.h include/global.h include/proc.h include/keyboard.h include/keymap.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o:kernel/protect.c include/const.h include/type.h include/global.h include/proto.h include/protect.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o:kernel/tty.c include/const.h include/type.h include/protect.h include/proto.h include/global.h include/proc.h include/tty.h include/console.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/start.o:kernel/start.c include/const.h include/type.h include/protect.h include/proto.h include/global.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/main.o:kernel/main.c include/const.h include/type.h include/tty.h include/fs.h include/protect.h include/proto.h include/global.h include/proc.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/global.o:kernel/global.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o:kernel/i8259.c include/const.h include/type.h include/protect.h include/proto.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o:kernel/clock.c include/const.h include/type.h include/protect.h include/proto.h include/proc.h include/global.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/shell.o:kernel/shell.c include/const.h include/type.h include/tty.h include/fs.h include/protect.h include/proto.h include/global.h include/proc.h
			$(CC) $(CFLAGS) -o $@ $<

fs/fs.o:fs/fs.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

fs/open.o:fs/open.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

fs/read_write.o:fs/read_write.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

fs/unlink.o:fs/unlink.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h

			$(CC) $(CFLAGS) -o $@ $<

fs/stat.o:fs/stat.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<
 
lib/usr/open.o:lib/usr/open.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

lib/usr/rdwt.o:lib/usr/rdwt.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

lib/usr/unlink.o:lib/usr/unlink.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
	        $(CC) $(CFLAGS) -o $@ $<

lib/usr/fork.o:lib/usr/fork.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

lib/usr/exit.o:lib/usr/exit.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

lib/usr/wait.o:lib/usr/wait.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
	        $(CC) $(CFLAGS) -o $@ $<

lib/usr/getpid.o:lib/usr/getpid.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

lib/usr/exec.o:lib/usr/exec.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

lib/usr/stat.o:lib/usr/stat.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/hd.o:kernel/hd.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h

lib/klib.o:lib/klib.c include/proto.h include/const.h include/type.h
			$(CC) $(CFLAGS) -o $@ $<

lib/string.o:lib/string.c
			$(CC) $(CFLAGS) -o $@ $<

lib/misc.o:lib/misc.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h
			$(CC) $(CFLAGS) -o $@ $<

mm/mm.o:mm/mm.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

mm/forkexit.o: mm/forkexit.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

mm/exit.o:mm/exit.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
	        $(CC) $(CFLAGS) -o $@ $<

mm/exec.o:mm/exec.c include/const.h include/type.h include/protect.h include/global.h include/proc.h include/tty.h include/console.h include/hd.h include/fs.h
			$(CC) $(CFLAGS) -o $@ $<

kernel/kernel.bin:$(OBJS)
			ld -s -m elf_i386 -Ttext 0x30400 -o kernel/kernel.bin kernel/kernel.o kernel/main.o kernel/shell.o kernel/printf.o lib/usr/stat.o lib/usr/exec.o lib/usr/getpid.o lib/usr/exit.o lib/usr/wait.o \
			lib/usr/fork.o lib/usr/unlink.o lib/usr/rdwt.o lib/usr/open.o mm/exec.o mm/exit.o mm/mm.o mm/forkexit.o fs/stat.o fs/unlink.o fs/open.o fs/read_write.o fs/fs.o \
			lib/misc.o kernel/systask.o kernel/tty.o kernel/console.o kernel/keyboard.o kernel/hd.o kernel/clock.o kernel/proc.o kernel/syscall.o \
			kernel/start.o kernel/protect.o kernel/i8259.o lib/string.o lib/cstring.o lib/klib.o lib/kliba.o kernel/global.o


start:		   
			$(BO)				