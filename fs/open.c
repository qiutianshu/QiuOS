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


/*设置inode位图，返回inode编号*/
PRIVATE int alloc_imap_bit(int dev){
	int inode_nr = 0;
	int i,j,k;
	int imap_blk0_nr = 2;								//inode_map起始扇区 = 引导扇区+超级块
	struct super_block* sb = get_super_block(dev);

	for(i = 0; i < sb->nr_imap_sects; i++){
		rw_sector(DEV_READ, dev, imap_blk0_nr + i, 512, TASK_FS, fsbuf);			//读入一个扇区的inode_map
		for(j = 0; j < 512; j++){
			if(fsbuf[j] == 0xff)
				continue;
			for(k = 0; ((fsbuf[j] >> k) & 1);k++){}
			fsbuf[j] |= 1 << k;
			inode_nr = (i * 512 + j) * 8  + k;
			rw_sector(DEV_WRITE, dev, imap_blk0_nr + i, 512,TASK_FS, fsbuf);
			return inode_nr;
		}
	}
	return 0;
}

/*设置sector位图，返回文件第一个扇区编号*/
PRIVATE int alloc_smap_bit(int dev, int size_in_sector){
	int free_sect_nr = 0;
	int nr_sects = size_in_sector;
	int i,j,k;
	struct super_block* sb = get_super_block(dev);
	int smap_blk0_nr = 2 + sb->nr_imap_sects;				//sector map起始扇区 = 引导扇区 + 超级块 + inode map
	for(i = 0; i < sb->nr_smap_sects; i++){
		rw_sector(DEV_READ, dev, smap_blk0_nr + i, 512,TASK_FS, fsbuf);
		for(j = 0; j < 512 && nr_sects > 0; j++){
			k = 0;
			if(!free_sect_nr){
				if(fsbuf[j] == 0xff)
					continue;
				for(k = 0; (fsbuf[j] >> k) & 1; k++){}
				free_sect_nr = (i * 512 + j) * 8 + k - 1 + sb->n_1st_sect;
			}
			for(; k < 8; k++){
				fsbuf[j] |= 1 << k;
				if(--nr_sects == 0)							//剩下扇区
					break;
			}
		}
		if(free_sect_nr)
			rw_sector(DEV_WRITE, dev, smap_blk0_nr + i, 512,TASK_FS, fsbuf);
		if(nr_sects == 0)
			break;
	}
	return free_sect_nr;
}

/*在inode缓冲区新建一个inode并写入磁盘*/
/*
*		次设备号
*		inode编号
*		文件起始扇区号
*/
PRIVATE struct inode* new_inode(int dev, int inode_nr, int start_sect){
	struct inode* new_inode = get_inode(dev, inode_nr);		//首先在inode缓冲区找到空位
	new_inode->i_mode = I_REGULAR;
	new_inode->i_size = 0;									//初始化大小为0
	new_inode->i_start_sect = start_sect;					//文件起始扇区号
	new_inode->i_nr_sects = NR_DEFAULT_FILE_SECTS;			//系统为每个文件预留扇区
	new_inode->i_dev = dev;									//本inode属于哪个设备
	new_inode->i_cnt = 1;									//已被进程占用
	new_inode->i_num = inode_nr;							//本inode位置

	sync_inode(new_inode);									//将缓冲区内容写入磁盘

	return new_inode;
}

PRIVATE void new_dir_entry(struct inode* dir_inode, int inode_nr, char* filename){
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + 512) / 512;
	int i,j;
	int m = 0;
	struct dir_entry* newinode = 0;
	struct dir_entry* pde;
	for(i = 0; i < nr_dir_blks; i++){
		rw_sector(DEV_READ, dir_inode->i_dev, dir_blk0_nr + i, 512,TASK_FS, fsbuf);
		pde = (struct dir_entry*)fsbuf;

		for(j = 0 ;j < 512 / DIR_ENTRY_SIZE; j++,pde++){
			if(pde->inode_nr == 0){				//找到一个空槽位
				newinode = pde;
				break;
			}
		}
		if(newinode)			//找到插入点
			break;
	}
	if(!newinode)				//inode在磁盘上已满
		panic("No more space for new inode on the disk!");

	dir_inode->i_size +=DIR_ENTRY_SIZE;

	newinode->inode_nr = inode_nr;
	strcpy(newinode->name, filename);
	rw_sector(DEV_WRITE, dir_inode->i_dev, dir_blk0_nr + i, 512, TASK_FS, fsbuf);			//写入根目录
	sync_inode(dir_inode);					//根目录inode更新
}

PUBLIC int do_open(){
	int fd = -1;
	int i;
	int inode_nr;
	char pathname[MAX_PATH];
	int flag		= fs_msg.FLAGS;
	int name_len	= fs_msg.NAME_LEN;
	int src 		= fs_msg.source;

	assert(name_len < MAX_PATH);

	phy_cpy(va2la(TASK_FS, pathname), va2la(src, fs_msg.PATHNAME), name_len);			//从TaskA 拷贝字符串到FS
	pathname[name_len] = 0;

	for(i = 0; i < NR_FILES; i++){				//找文件句柄
		if(caller->filp[i] == 0){
			fd = i;
			break;
		}
	}
	
	if(fd < 0 || fd >= NR_FILES)
		panic("filp is full!!(PID: %d)", proc2pid(caller));

	for(i = 0; i < NR_FILE_DESC; i++){
		if(file_desc_table[i].fd_inode == 0)
			break;
	}
	
	if(i >= NR_FILE_DESC)
		panic("file_desc_table is full!!(PID: %d)",proc2pid(caller));

	inode_nr = search_file(pathname);
	
	struct inode* pin = 0;
	if(flag & O_CREATE){
		if(inode_nr){
			printl("file exists!\n ");
			return -1;
		}
		else
			pin = create_file(pathname, flag);
	}
	else{
		assert(flag & O_RW);

		char filename[MAX_PATH];
		struct inode* dir_inode;
		if(strip_path(filename, pathname, &dir_inode) != 0)				//路径转换为文件名 返回文件目录的inode
			return -1;

		pin = get_inode(dir_inode->i_dev, inode_nr);						//文件inode装入inode_table

	}
	/*装载顺序 filp、inode_table、file_desc_table*/
	if(pin){

		/*设置filp 、 file_desc_table 、inode_table*/
		caller->filp[fd] = &file_desc_table[i];
		file_desc_table[i].fd_inode = pin;								//指向inode_table中的元素
		file_desc_table[i].fd_mode = flag;	
		file_desc_table[i].fd_cnt = 1;				 			
		file_desc_table[i].fd_pos  = 0;
		
		int imode = pin->i_mode & I_TYPE_MASK;

		if(imode == I_CHAR_SPECIAL){
			MESSAGE msg;
			msg.type = DEV_OPEN;
											//字符设备起始扇区为其设备号
			msg.DEVICE = MINOR(pin->i_start_sect);
			assert(MAJOR(pin->i_start_sect) == TASK_TTY);
			msg.PROC_NR = fs_msg.source;

			send_recv(BOTH, dd[MAJOR(pin->i_start_sect)].drv_pid, &msg); 
		}
		else if(imode == I_DIRECTORY){
			assert(pin->i_num == ROOT_INODE);
		}
		else
			assert(pin->i_mode == I_REGULAR);					//filp 、 file_desc_table 、 inode_table分配完毕就算文件打开成功
	}
	else
		return -1;
	return fd;
}

/*关闭文件操作*/
PUBLIC int do_close(){
	int i = 0;
	int fd = fs_msg.FD;
	put_inode(caller->filp[fd]->fd_inode);				//inode计数减一

	caller->filp[fd]->fd_inode = 0;
	caller->filp[fd] = 0;
	return 0;
}

/*PUBLIC int do_close(){

}*/

/*由文件路径返回文件名，同时返回文件所在根目录的inode*/
PUBLIC int strip_path(char*filename, char* path, struct inode** ppinode){
	char* s = path;
	char* t = filename;

	if(s == 0)
		return -1;

	if(*s == '/')
		s++;

	while(*s){
		if(*s == '/')
			return -1;

		*t++ = *s++;
		if(t - filename >= MAX_NAME)
			break;
	}
	*t = 0;
	*ppinode = root_inode;						//返回根目录inode
	return 0;
}

/*搜索文件是否存在，成功返回文件inode编号，失败返回0*/
PUBLIC int search_file(char* path){
	int i,j,k;
	int flag = 0;
	int m = 0;
	char filename[MAX_PATH];
	memset(filename, 0, MAX_NAME);
	struct inode* dir_inode;
	if(strip_path(filename, path, &dir_inode) != 0)
		return 0;

	if(filename[0] == 0)						//根目录
		return dir_inode->i_num;

	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + 512) / 512;	//根目录占用扇区大小
	int nr_dir_ents = dir_inode->i_size / DIR_ENTRY_SIZE;
	struct dir_entry* pde;
	for(i = 0; i < nr_dir_blks; i++){

		rw_sector(DEV_READ, dir_inode->i_dev, dir_inode->i_start_sect + i, 512,TASK_FS, fsbuf);
		pde = (struct dir_entry*)fsbuf;
		for(j = 0; j < nr_dir_ents; j++,pde++){
			for(k = 0; k < MAX_NAME; k++){
				if(filename[k] != pde->name[k]){
					flag = 1;
					break;
				}													//比较文件名
			}
			if(!flag)
					return pde->inode_nr;							//比较成功，返回文件inode
			flag = 0;
			if(++m >= nr_dir_ents)
				break;
		}
		if(m >= nr_dir_ents)
			break;
	}
	return 0;														//文件未找到
}

PUBLIC struct inode* create_file(char* path, int flag){
	char filename[MAX_PATH];
	struct inode* dir_inode;
	if(strip_path(filename, path, &dir_inode) != 0)
		return 0;

	int inode_nr = alloc_imap_bit(dir_inode->i_dev);		                        //inode_map 置位	
	int free_sect_nr = alloc_smap_bit(dir_inode->i_dev, NR_DEFAULT_FILE_SECTS);		//设置sector_map，返回文件起始扇区号
	struct inode* newinode = new_inode(dir_inode->i_dev, inode_nr, free_sect_nr);	//设置inode
	new_dir_entry(dir_inode, newinode->i_num, filename);							//写入dir_entry
	return newinode;
}

/*获得指定编号的inode并返回inode_table中的指针*/
/*
*		设备号
*		inode编号
*/
PUBLIC struct inode* get_inode(int dev, int num){
	if(num == 0)
		return 0;
	struct inode* p;
	struct inode* q = 0;
	for(p = inode_table; p < &inode_table[NR_INODES]; p++){
		if(p->i_cnt){
			if(p->i_dev == dev && p->i_num == num){
				p->i_cnt++;						//占用数加一
				return p;						//已经在inode_table中，直接返回
			}
		}
		else
			if(!q)								//q指向第一个空槽位
				q = p;
	}
	if(!q)
		panic("inode_table is full!");

	q->i_cnt = 1;
	q->i_dev = dev;
	q->i_num = num;

	struct super_block* sb = get_super_block(dev);
	int blk_nr = 2 + sb->nr_smap_sects + sb->nr_imap_sects + (num - 1) / (512 / INODE_SIZE); // 定位inode所在扇区
	rw_sector(DEV_READ, dev, blk_nr, 512,TASK_FS, fsbuf);
	struct inode* pnode = (struct inode*)((u8*)fsbuf + ((num - 1) % (512 / INODE_SIZE)) * INODE_SIZE);
	q->i_mode = pnode->i_mode;
	q->i_size = pnode->i_size;
	q->i_start_sect = pnode->i_start_sect;
	q->i_nr_sects = pnode->i_nr_sects;
	return q;
}

/*减少对inode的引用，当引用减为0则代表文件已被弃用并可被新inode覆盖*/
PUBLIC void put_inode(struct inode* pnode){
	assert(pnode->i_cnt > 0);
	pnode->i_cnt--;
}

/*当inode_table中的内容有变化时，写回磁盘*/
PUBLIC void sync_inode(struct inode* pnode){
	struct inode* p;
	struct super_block* sb = get_super_block(pnode->i_dev);
	int blk_nr = 2 + sb->nr_imap_sects + sb->nr_smap_sects + (pnode->i_num - 1) / (512 / INODE_SIZE);		//定位inode所在扇区
	rw_sector(DEV_READ, pnode->i_dev, blk_nr, 512,TASK_FS, fsbuf);
	/*定位要修改的inode*/
	struct inode* ppnode = (struct inode*)((u8*)fsbuf + ((pnode->i_num - 1) % (512 / INODE_SIZE)) * INODE_SIZE);
	ppnode->i_mode = pnode->i_mode;
	ppnode->i_size = pnode->i_size;
	ppnode->i_start_sect = pnode->i_start_sect;
	ppnode->i_nr_sects = pnode->i_nr_sects;
	rw_sector(DEV_WRITE, pnode->i_dev, blk_nr, 512, TASK_FS, fsbuf);				//写回磁盘
}
