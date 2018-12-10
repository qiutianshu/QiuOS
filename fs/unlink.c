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

/*文件删除操作
*
* 1、清除inode map相应位
* 2、清除sector map相应位
* 3、inode清零
* 4、dir entry清零
* 5、删除tty报错
*
* 成功返回0， 失败返回-1
*/

PUBLIC int do_unlink(){
	char filepath[MAX_PATH];
	char filename[MAX_NAME];
	int path_len = fs_msg.NAME_LEN;
	int src = fs_msg.source;
	int inode_nr = 0;
	int i,j;
	int k = 0;
	int flag1 = 0;					//首字节是否被归零
	struct inode* dir_inode;
	struct inode* p;
	struct super_block* sb;
	phy_cpy(va2la(TASK_FS, filepath), va2la(src, fs_msg.PATHNAME), path_len);

	filepath[path_len] = 0;
	if(!strcmp(filepath, "/")){						//试图删除根目录
		printl("can not delete root directory!\n ");
		return -1;
	}

	inode_nr = search_file(filepath);				//文件不存在
	if(!inode_nr){
		printl(" %s does not exist!\n ",filepath);
		return -1;
	}

	if(strip_path(filename, filepath, &dir_inode) != 0)//得到根目录inode进而得到设备号
		return -1;

	p = get_inode(dir_inode->i_dev, inode_nr);		//得到文件inode

	if(p->i_mode != I_REGULAR){
		printl("can not delete %s\n", filepath);
		return -1;
	}

	if(p->i_cnt > 1){
		printl("file in used!\n ");
		return -1;
	}

	sb = get_super_block(dir_inode->i_dev);

	/*清除inode map相应位*/
	int bytes_off = inode_nr / 8;
	int bits_off = inode_nr % 8;
	rw_sector(DEV_READ, p->i_dev, 2, 512, TASK_FS, fsbuf);
	fsbuf[bytes_off] &= ~(1 << bits_off);
	rw_sector(DEV_WRITE, p->i_dev, 2, 512, TASK_FS, fsbuf);

	/*清除sector map相应位*/
	int init_bit = p->i_start_sect + 1 - sb->n_1st_sect;	//sector中的起始位，相对于第1位
	int init_byte = init_bit / 8;
	int bits_left = p->i_nr_sects;							//文件占用扇区数，默认2048
	int bytes_cnt = (bits_left - (8 - init_bit % 8)) / 8;	//整字节数
	int init_sector = 2 + sb->nr_imap_sects + init_byte / 512;						
	for(i = 0; i <= NR_DEFAULT_FILE_SECTS / 4096 + 1; i++){ //考虑到跨扇区
		rw_sector(DEV_READ, dir_inode->i_dev, init_sector + i, 512, TASK_FS, fsbuf);

		if(!flag1){											//处理sector map首字节
			/*之所以j没有直接从1开始，并且考虑bits_left，是为了后面文件系统的改进*/
			for(j = init_bit % 8; (j < 8) && bits_left > 0; j++, bits_left--)  
				fsbuf[init_byte % 512] &= ~(1 << j);
			flag1 = 1;
			j = init_byte % 512 + 1;
		}
		/*第二字节到倒数第二个字节*/
		for(; k < bytes_cnt; k++,j++,bits_left-=8){
			if(j == 512){
				rw_sector(DEV_WRITE, dir_inode->i_dev, init_sector + i, 512, TASK_FS, fsbuf);
				break;
			}
			fsbuf[j] = 0;
		}
		if(k < bytes_cnt)
			j = 0;
		/**最后一个字节，最后一个扇区**/
		if(k == bytes_cnt){
			fsbuf[j] &= (~0) << bits_left;
			rw_sector(DEV_WRITE, dir_inode->i_dev, init_sector + i, 512, TASK_FS, fsbuf);
			break;
		}
	}

	/*清除inode*/
	p->i_mode = 0;
	p->i_size = 0;
	p->i_start_sect = 0;
	p->i_nr_sects = 0;
	sync_inode(p);
	put_inode(p);

	/*清除dir entry*/
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = dir_inode->i_size / 512 + 1;
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;	//包括已删除的未使用的
	int dir_size = 0;											//记录修改过的根目录大小
	int flag  = 0;	
	int m = 0;											//dir entry 是否找到
	struct dir_entry* pde = 0;

	for(i = 0; i < nr_dir_blks; i++){
		rw_sector(DEV_READ, dir_inode->i_dev, dir_blk0_nr + i, 512, TASK_FS, fsbuf);
		pde = (struct dir_entry*)fsbuf;
		for(j = 0; j < 512/DIR_ENTRY_SIZE; j++, pde++){
			++m;
			if(pde->inode_nr == inode_nr){
				memset(pde, 0, DIR_ENTRY_SIZE);
				rw_sector(DEV_WRITE, dir_inode->i_dev, dir_blk0_nr + i, 512, TASK_FS, fsbuf);
				flag = 1;
				break;
			}
			if(pde->inode_nr != 0)
				dir_size += DIR_ENTRY_SIZE;
		}
		if(flag)
			break;
	}
	assert(flag);

	if(m == nr_dir_entries){			//删除最后一个
		dir_inode->i_size = dir_size;
		sync_inode(dir_inode);
	}
	return 0;
}