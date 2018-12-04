#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "hd.h"
#include "fs.h"
#include "global.h"

PRIVATE void rw_sector(int type, int dev, int pos_in_sector, int bytes, int proc, void* buf){
	MESSAGE msg;
	msg.type 		= type;
	msg.DEVICE 		= MINOR(dev);
	msg.POSITION	= pos_in_sector;
	msg.BUF 		= buf;
	msg.COUNT		= bytes;
	msg.PROC_NR		= proc;
	send_recv(BOTH, dd[MAJOR(dev)].drv_pid, &msg);
}

PRIVATE void mkfs(){
	MESSAGE msg;
	int i,j;
	int bits_per_sect = 512 * 8;
	struct part_info geo;

	/*获取设备信息*/
	msg.type = DEV_IOCTL;
	msg.DEVICE = MINOR(ROOT_DEV);
	msg.REQUEST = DIOCTL_GET_GEO;
	msg.BUF = &geo;
	msg.PROC_NR = TASK_FS;
	send_recv(BOTH, dd[MAJOR(ROOT_DEV)].drv_pid, &msg);

	printl("dev size: %d sectors\n ", geo.size);

	/*设置超级块参数*/
	struct super_block sb;
	sb.magic 			= MAGIC_V1;
	sb.nr_inodes		= bits_per_sect;			//最大4096个文件
	sb.nr_inode_sects	= sb.nr_inodes * INODE_SIZE / 512; // 512;
	sb.nr_sects 		= geo.size;					//设备大小
	sb.nr_imap_sects	= 1;						//inode map扇区数
	sb.nr_smap_sects	= sb.nr_sects / bits_per_sect + 1;		//sector maps 占用扇区数
	sb.n_1st_sect 		= 2 + sb.nr_imap_sects + sb.nr_inode_sects + sb.nr_smap_sects;		//数据区第一个扇区号
	sb.root_inode		= ROOT_INODE;
	sb.inode_size 		= INODE_SIZE;		

	struct inode x;
	sb.inode_isize_off	= (int)&x.i_size - (int)&x;
	sb.inode_start_off	= (int)&x.i_start_sect - (int)&x;

	sb.dir_ent_size		= DIR_ENTRY_SIZE;

	struct dir_entry y;
	sb.dir_ent_inode_off	= (int)&y.inode_nr - (int)&y;
	sb.dir_enr_fname_off	= (int)&y.name	- (int)&y;

	memset(fsbuf, 0x90, 512);
	memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

	rw_sector(DEV_WRITE, ROOT_DEV, 1, 512, TASK_FS, fsbuf);			//根设备第一扇区写入超级块

	/*设置inode map*/
	memset(fsbuf, 0, 512);
	for(i = 0; i < NR_CONSOLES + 2; i++)
		fsbuf[0] |= 1 << i;

	assert(fsbuf[0] == 0x1f);

	rw_sector(DEV_WRITE, ROOT_DEV, 2, 512, TASK_FS, fsbuf);			//根设备第二扇区写入inode map

	/*设置sector map*/
	memset(fsbuf, 0, 512);
	for(i = 0; i < (NR_DEFAULT_FILE_SECTS + 1) / 8; i++)			//sector map第一位对应于数据区起始扇区，根目录位于数据区
		fsbuf[i] = 0xff;											//满8个扇区的先置1

	for(j = 0; j < (NR_DEFAULT_FILE_SECTS + 1) % 8; j++)
		fsbuf[i] |= (1 << j);

	rw_sector(DEV_WRITE, ROOT_DEV, 2 + sb.nr_imap_sects, 512, TASK_FS, fsbuf);

	memset(fsbuf, 0, 512);
	for(i = 1; i < sb.nr_smap_sects; i++)
		rw_sector(DEV_WRITE, ROOT_DEV, 2 + sb.nr_imap_sects + i, 512, TASK_FS, fsbuf);	//sector map 剩余部分归零

	/*设置根目录inode*/
	memset(fsbuf, 0, 512);
	struct inode* pi = (struct inode*)fsbuf;
	pi->i_mode 			= I_DIRECTORY;
	pi->i_size 			= DIR_ENTRY_SIZE * 4;  	//4个文件 . tty0 tty1 tty2
	pi->i_start_sect	= sb.n_1st_sect;		//根目录起始扇区
	pi->i_nr_sects    	= NR_DEFAULT_FILE_SECTS;

	for(i = 0; i < NR_CONSOLES; i++){			//设置dev_tty0 - dev_tty2
		pi 				= (struct inode*)(fsbuf + (INODE_SIZE) * (i + 1));
		pi->i_mode 		= I_CHAR_SPECIAL;		//字符设备
		pi->i_size  	= 0;
		pi->i_start_sect=MAKE_DEVICE(DEV_CHAR_TTY, i);		//字符设备号
		pi->i_nr_sects  = 0;
	}

	rw_sector(DEV_WRITE, ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects, 512, TASK_FS, fsbuf);
	/*设置根目录项*/
	memset(fsbuf, 0, 512);
	struct dir_entry *pde = (struct dir_entry*)fsbuf;
	pde->inode_nr = 1;
	strcpy(pde->name, ".");					//根目录

	for(i = 0; i < NR_CONSOLES; i++){
		pde++;
		pde->inode_nr = i + 2;				//dev_tty0
		sprintf(pde->name, "dev_tty%d", i);
	}
	printl("1st : %x\n ", sb.n_1st_sect * 512);
	rw_sector(DEV_WRITE, ROOT_DEV, sb.n_1st_sect, 512, TASK_FS, fsbuf);
}

PRIVATE	void init_fs(){
	MESSAGE msg;
	msg.type = DEV_OPEN;
	msg.DEVICE = MINOR(ROOT_DEV);
	send_recv(BOTH, dd[MAJOR(ROOT_DEV)].drv_pid, &msg);

	mkfs();
}

PUBLIC void task_fs(){
	init_fs();
	spin("FS loop");
}

