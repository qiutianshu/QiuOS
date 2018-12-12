#ifndef _ELF_H_
#define _ELF_H_

#define EI_NIDENT 			16

typedef unsigned int 		Elf32_Addr;				//无符号程序地址
typedef unsigned short		Elf32_Half;				//无符号中等大小整数
typedef unsigned int 		Elf32_Off;				//无符号文件偏移
typedef int 				Elf32_Sword;			//有符号大整数
typedef unsigned int 		Elf32_Word;				//无符号大整数

typedef struct {
	unsigned char		e_ident[EI_NIDENT];			/*ELF的一些标识信息，固定值*/
	Elf32_Half 			e_type;						/*目标文件类型：1-可重定位文件，2-可执行文件，3-共享目标文件等*/
	Elf32_Half          e_machine;                  /*文件的目标体系结构类型：3-intel 80386*/
	Elf32_Word 			e_version;                  /*目标文件版本：1-当前版本*/
	Elf32_Addr 			e_entry;                    /*程序入口的虚拟地址，如果没有入口，可为0*/
	Elf32_Off			e_phoff;                    /*程序头表(segment header table)的偏移量，如果没有，可为0*/
	Elf32_Off			e_shoff;                    /*节区头表(section header table)的偏移量，没有可为0*/
	Elf32_Word 			e_flags;                    /*与文件相关的，特定于处理器的标志*/
	Elf32_Half 			e_ehsize;                   /*ELF头部的大小，单位字节*/
	Elf32_Half 			e_phentsize;                /*程序头表每个表项的大小，单位字节*/
	Elf32_Half			e_phnum;                    /*程序头表表项的个数*/
	Elf32_Half			e_shentsize;                /*节区头表每个表项的大小，单位字节*/
	Elf32_Half			e_shnum;                   /*节区头表表项的数目*/
	Elf32_Half			e_shstrndx;                 /*某些节区中包含固定大小的项目，如符号表。对于这类节区，此成员给出每个表项的长度字节数。*/
}Elf32_Ehdr;

typedef struct{   
    Elf32_Word p_type;								/*segment的类型：PT_LOAD= 1 可加载的段*/ 
    Elf32_Off p_offset;							    /*从文件头到该段第一个字节的偏移*/
    Elf32_Addr p_vaddr;							    /*该段第一个字节被放到内存中的虚拟地址*/
    Elf32_Addr p_paddr;                             /*在linux中这个成员没有任何意义，值与p_vaddr相同*/
    Elf32_Word p_filesz;                            /*该段在文件映像中所占的字节数*/
    Elf32_Word p_memsz;                             /*该段在内存映像中占用的字节数*/
    Elf32_Word p_flags;                             /*段标志*/
    Elf32_Word p_align;                             /*p_vaddr是否对齐*/
} Elf32_phdr;

typedef struct{
	Elf32_Word sh_name;								/*节区名称*/
	Elf32_Word sh_type;								/*节区类型：PROGBITS-程序定义的信息，NOBITS-不占用文件空间(bss),REL-重定位表项*/
	Elf32_Word sh_flags;							/*每一bit位代表一种信息，表示节区内的内容是否可以修改，是否可执行等信息*/
	Elf32_Addr sh_addr;								/*如果节区将出现在进程的内存影响中，此成员给出节区的第一个字节应处的位置*/
	Elf32_Off sh_offset;							/*节区的第一个字节与文件头之间的偏移*/
	Elf32_Word sh_size;								/*节区的长度，单位字节，NOBITS虽然这个值非0但不占文件中的空间*/
	Elf32_Word sh_link;								/*节区头部表索引链接*/
	Elf32_Word sh_info;								/*节区附加信息*/
	Elf32_Word sh_addralign;						/*节区带有地址对齐的约束*/
	Elf32_Word sh_entsize;							/*某些节区中包含固定大小的项目，如符号表，那么这个成员给出其固定大小*/
}Elf32_Shdr;

#endif