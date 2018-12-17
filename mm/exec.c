#include "type.h"
#include "const.h"
#include "elf.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "hd.h" 
#include "global.h"

PUBLIC int do_exec(){
	char pathname[MAX_PATH];
	int src = mm_msg.source;
	int path_len = mm_msg.NAME_LEN;
	assert(path_len < MAX_PATH);
	int argv_len = mm_msg.BUF_LEN;				       //为指针数组预留的空间（参数个数 × 8）
	char ** p = va2la(src, mm_msg.BUF);
	int argc = 0;										//参数个数
	int i;
	/*main函数执行前的栈，位于进程空间顶端0x400*/
	char** init_stack = (char**)va2la(src, (PROC_IMG_DEFAULT_SIZE - INIT_STACK));	
	*((int*)init_stack + argv_len) = 0; 			//指针数组与字符串之间以0为间隔
	argv_len += sizeof(char*);

	char** ff = init_stack;
/*	
-------------------+--+--+--+--+--------+--------------------+-------+---------------+
  ......           |  |  |  |  | ....|  |  str1|      |str3  |       |  ...          |
                   |  |  |  |  |     | 0|      |str2  |      |str4   |               |
-------------------+-++-++-+++-+-----+-----+------+---+---+------+---+---------------+
                     |  |  | |             ^      ^       ^      ^
                     |  |  | +-----------------------------------+
                     |  |  +------------------------------+
                     |  |                  |      |
                     |  +-------------------------+
                     +---------------------+
*/
	for(; *p !=0; p++){										 //遍历参数列表
		phy_cpy((char*)init_stack + argv_len, va2la(src, *p), strlen(va2la(src, *p)));      //复制参数
		*ff++ = PROC_IMG_DEFAULT_SIZE - INIT_STACK + argv_len;
		argv_len += strlen(va2la(src,*p));
		*((char*)init_stack + argv_len) = 0;
		argv_len++;
		argc++;
	}

	/*复制代码至文件系统缓冲区*/
	phy_cpy(va2la(TASK_MM, pathname), va2la(src,mm_msg.PATHNAME), path_len);
	pathname[path_len] = 0;
	int fd = open(pathname, O_RW);
	if(fd == -1){
		printf("do_exec open file %s error!\n", pathname);
		return -1;
	}

	struct status s;

	if(stat(pathname, &s) == -1){
		printf("do_exec::stat() returns error! %s\n ",pathname);
		return -1;
	}

	assert(s.size < MMBUF_SIZE);
	read(fd, mmbuf, s.size);
	close(fd); 

	/*将代码从缓冲区复制到进程空间*/
	Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*)mmbuf;
	for(i = 0; i < elf_hdr->e_phnum; i++){
		Elf32_Phdr* prog_hdr = (Elf32_Phdr*)(mmbuf + elf_hdr->e_phoff + i * elf_hdr->e_phentsize);
		if(prog_hdr->p_type == 1){					//PT_LOAD = 1 可加载的段
			assert(prog_hdr->p_vaddr + prog_hdr->p_memsz < PROC_IMG_DEFAULT_SIZE);
			phy_cpy(va2la(src, prog_hdr->p_vaddr), 
					va2la(TASK_MM, mmbuf + prog_hdr->p_offset),
					prog_hdr->p_memsz);
		}
	}

	/*修改进程表*/
	proc_table[src].regs.ecx = argc;
	proc_table[src].regs.eax = PROC_IMG_DEFAULT_SIZE - INIT_STACK;

	proc_table[src].regs.eip = elf_hdr->e_entry;
	proc_table[src].regs.esp = PROC_IMG_DEFAULT_SIZE - INIT_STACK;

	strcpy(proc_table[src].p_name, pathname);

	return 0;
}