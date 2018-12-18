#include "type.h"
#include "const.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "hd.h" 
#include "global.h"

PUBLIC void rw_sector(int type, int dev, int pos_in_sector, int bytes, int proc, void* buf){
	MESSAGE msg;
	msg.type 		= type;
	msg.DEVICE 		= MINOR(dev);
	msg.POSITION	= pos_in_sector;
	msg.BUF 		= buf;
	msg.COUNT		= bytes;
	msg.PROC_NR		= proc;
	send_recv(BOTH, dd[MAJOR(dev)].drv_pid, &msg);
}

/*读取指定设备的超级块进入super_block*/
PUBLIC void read_super_block(int dev){
	int i;
	MESSAGE msg;
	msg.type = DEV_READ;
	msg.POSITION = 1;			//第一扇区超级块
	msg.BUF = fsbuf;
	msg.DEVICE = MINOR(dev);
	msg.COUNT = 512;
	msg.PROC_NR = TASK_FS;

	send_recv(BOTH, dd[MAJOR(dev)].drv_pid, &msg);

	for(i = 0; i < NR_SUPER_BLOCK; i++){
		if(super_block[i].sb_dev == NO_DEV)
			break;
	}
	if(i == NR_SUPER_BLOCK)
		panic("super_block table is full!!");

	struct super_block* sb = (struct super_block*)fsbuf;
	super_block[i] = *sb;
	super_block[i].sb_dev = dev;
}

/*从super_block table中获得指定block*/
PUBLIC struct super_block* get_super_block(int dev){
	int i;
	for(i = 0; i < NR_SUPER_BLOCK; i++){
		if(super_block[i].sb_dev == dev)
			return &super_block[i];
	}
	panic("super_block of dev %d not found!!",dev);
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
	send_recv(BOTH, dd[MAJOR(ROOT_DEV)].drv_pid, &msg);			//获得根设备起始扇区和扇区数

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

	printl("nr_smap_sects: %d\n",sb.nr_smap_sects);	

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

	printl("super_block start at : %x \n inode map start at : %x \n sector map start at : %x \n inode array start at : %x \n root directory start at : %x \n data block start at : %x\n", 
			(geo.base + 1) * 512, /*super block*/
			(geo.base + 1 + 1) * 512 , /*inode map*/
		    (geo.base + 2 + sb.nr_imap_sects) * 512, /*sector map*/
		    (geo.base + 2 + sb.nr_smap_sects + sb.nr_imap_sects) * 512,  /*inode array*/
		    (geo.base + 2 + sb.nr_smap_sects + sb.nr_imap_sects + sb.nr_inode_sects) * 512, /*root directory*/
		    (geo.base + 2 + sb.nr_smap_sects + sb.nr_imap_sects + sb.nr_inode_sects + NR_DEFAULT_FILE_SECTS) * 512); /*data block*/

	/*设置inode map*/
	memset(fsbuf, 0, 512);
	for(i = 0; i < NR_CONSOLES + 3; i++)
		fsbuf[0] |= 1 << i;

	assert(fsbuf[0] == 0x3f);

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

	/*安装cmd.tar*/
	int bit_off				 = INSTALL_FIRST_SECTOR - sb.n_1st_sect + 1;
	int bit_off_in_sector	 = bit_off % (8 * 512);	
	int byte_off_in_sector	 = bit_off_in_sector / 8;			//扇区内字节偏移
	int cur_sector			 = bit_off / (8 * 512);				//当前读写扇区
	int bits_left			 = INSTALL_NR_SECTORS;				//剩余位数
	int bytes_left;												//剩余字节数

	//printl("sector map of cmd.tar start at:%x\n", 512 * (geo.base + 2 + sb.nr_imap_sects + cur_sector) + byte_off_in_sector);

	rw_sector(DEV_READ, ROOT_DEV, 2 + sb.nr_imap_sects + cur_sector, 512, TASK_FS, fsbuf);

	if(fsbuf[byte_off_in_sector] != 0xff){						//填充不满1字节的部分
		for(i = (bit_off_in_sector % 8); i < 8; i++){			
			fsbuf[byte_off_in_sector] |= 1 << i;
			bits_left--;
		}	
		byte_off_in_sector++;	  
	}
	
	bytes_left = bits_left / 8;

	while(bytes_left){											//填充整字节
		if(byte_off_in_sector == 512){							//满1个扇区写回磁盘
			rw_sector(DEV_WRITE, ROOT_DEV, 2 + sb.nr_imap_sects + cur_sector,512, TASK_FS, fsbuf);
			byte_off_in_sector = 0;								//字节偏移归零
			cur_sector++;
			rw_sector(DEV_READ, ROOT_DEV, 2 + sb.nr_imap_sects + cur_sector, 512, TASK_FS, fsbuf);	//读入下一个扇区
		}
		else{
			fsbuf[byte_off_in_sector] = 0xff;					//写整字节
			byte_off_in_sector++;
			bytes_left--;
			bits_left = bits_left - 8;
		}												
	}

	if(byte_off_in_sector == 512){
		rw_sector(DEV_WRITE, ROOT_DEV, 2 + sb.nr_imap_sects + cur_sector,512, TASK_FS, fsbuf);
		byte_off_in_sector = 0;								
		cur_sector++;
		rw_sector(DEV_READ, ROOT_DEV, 2 + sb.nr_imap_sects + cur_sector, 512, TASK_FS, fsbuf);	//读入下一个扇区
	}
	
	i = 0;
	while(bits_left){											//填充最后一个字节
		fsbuf[byte_off_in_sector] |= 1 << i;
		i++;
		bits_left--;
	}		
	rw_sector(DEV_WRITE, ROOT_DEV, 2 + sb.nr_imap_sects + cur_sector,512, TASK_FS, fsbuf);//写回扇区


	/*设置根目录inode*/
	memset(fsbuf, 0, 512);
	struct inode* pi = (struct inode*)fsbuf;
	pi->i_mode 			= I_DIRECTORY;
	pi->i_size 			= DIR_ENTRY_SIZE * 5;  	//5个文件 . tty0 tty1 tty2 cmd.tar
	pi->i_start_sect	= sb.n_1st_sect;		//根目录起始扇区
	pi->i_nr_sects    	= NR_DEFAULT_FILE_SECTS;

	for(i = 0; i < NR_CONSOLES; i++){			//设置dev_tty0 - dev_tty2
		pi 				= (struct inode*)(fsbuf + (INODE_SIZE) * (i + 1));
		pi->i_mode 		= I_CHAR_SPECIAL;		//字符设备
		pi->i_size  	= 0;
		pi->i_start_sect=MAKE_DEVICE(DEV_CHAR_TTY, i);		//字符设备号
		pi->i_nr_sects  = 0;
	}

	pi = (struct inode*)(fsbuf + (INODE_SIZE * (NR_CONSOLES + 1)));		//设置cmd.tar的inode
	pi->i_mode = I_REGULAR;
	pi->i_size = INSTALL_NR_SECTORS * 512;
	pi->i_start_sect = INSTALL_FIRST_SECTOR;
	pi->i_nr_sects = INSTALL_NR_SECTORS;
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
	(++pde)->inode_nr = NR_CONSOLES + 2;
	strcpy(pde->name, "cmd.tar");
	rw_sector(DEV_WRITE, ROOT_DEV, sb.n_1st_sect, 512, TASK_FS, fsbuf);
}

PRIVATE int fs_fork(){
	int i;
	PROCESS* p = &proc_table[fs_msg.PID];
	for(i = 0; i < NR_FILES; i++){
		if(p->filp[i]){
			p->filp[i]->fd_cnt++;
			p->filp[i]->fd_inode->i_cnt++;
		}
	}
	return 0;
}

PRIVATE int fs_exit(){
	PROCESS * proc = &proc_table[fs_msg.PID];
	int i ;
	for(i = 0; i < NR_FILES; i++){
		if(proc->filp[i]){
			proc->filp[i]->fd_inode->i_cnt--;
			proc->filp[i]->fd_cnt--;
			if(proc->filp[i]->fd_cnt == 0)
				proc->filp[i]->fd_inode = 0;
			proc->filp[i] = 0;
		}
	}
	return 0;
}

PRIVATE	void init_fs(){
	int i;
	for(i = 0; i < NR_FILE_DESC; i++)									//初始化文件描述符表、inode_table，
		memset(&file_desc_table[i], 0, sizeof(struct file_desc));		//注意进程表中的文件打开表也要置零

	for(i = 0; i < NR_INODES; i++)
		memset(&inode_table[i], 0, sizeof(struct inode));

	struct super_block* sb = super_block;
	for(; sb < &super_block[NR_SUPER_BLOCK]; sb++)
		sb->sb_dev = NO_DEV;

	MESSAGE msg;														//打开硬盘
	msg.type = DEV_OPEN;
	msg.DEVICE = MINOR(ROOT_DEV);
	send_recv(BOTH, dd[MAJOR(ROOT_DEV)].drv_pid, &msg);

	/*读取超级块*/
	rw_sector(DEV_READ, ROOT_DEV, 1, 512, TASK_FS, fsbuf);				//如果发现magic字段
	sb = (struct super_block*)fsbuf;									//不刷新文件系统
	if(sb->magic != MAGIC_V1){
		printl("make file system...\n");
		mkfs();
	}

	read_super_block(ROOT_DEV);
	sb = get_super_block(ROOT_DEV);
	assert(sb->magic == MAGIC_V1);

	root_inode = get_inode(ROOT_DEV,ROOT_INODE);
}

PUBLIC void task_fs(){
	init_fs();

	while(1){ 
		send_recv(RECEIVE, ANY, &fs_msg);
		int src = fs_msg.source;
		caller = &proc_table[src];

		switch(fs_msg.type){
			case FILE_OPEN:
				fs_msg.FD = do_open();
				break;
			case FILE_CLOSE:
				fs_msg.RETVAL = do_close();
				break;
			case FILE_READ:
			case FILE_WRITE:
				fs_msg.RETVAL = do_rw();
				break;
			case FILE_DELETE:
				fs_msg.RETVAL = do_unlink();
				break;
			case RESUME_PROC:
				src = fs_msg.PROC_NR;
				break;
			case FORK:
				fs_msg.RETVAL = fs_fork();
				break;
			case EXIT:
				fs_msg.RETVAL = fs_exit();
				break;
			case FILE_STAT:
				fs_msg.RETVAL = fs_stat();
				break;
			default:
				break;
		}
		if(fs_msg.type != SUSPEND_PROC){
			fs_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &fs_msg);
		}
	}	
}

