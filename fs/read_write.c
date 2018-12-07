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

/*文件读写操作
*
*	返回操作的字节数
*/
PUBLIC int do_rw(){
	int fd 			= fs_msg.FD;
	int len			= fs_msg.COUNT;
	void* buf		= fs_msg.BUF;
	int src 		= fs_msg.source;
	int i,bytes;

	struct inode* p = caller->filp[fd]->fd_inode;

	/*检查文件描述符，防止出现未打开的读写*/
	assert((caller->filp[fd] >= file_desc_table) && \
			(caller->filp[fd] < &file_desc_table[NR_FILE_DESC]));

	if(!(caller->filp[fd]->fd_mode & O_RW))
		return -1;

	int f_pos 		= caller->filp[fd]->fd_pos;	
	int offset 		= f_pos % 512;	
	int mode		= p->i_mode & I_TYPE_MASK;

	if(mode == I_CHAR_SPECIAL){									//字符设备，有待完善
		/*		TODO		*/
	}
	else{														//普通文件
		assert((mode == I_REGULAR) || (mode == I_DIRECTORY));
		int start_sect	= p->i_start_sect + f_pos / 512;		//起始扇区
		int end_sect;
		if(fs_msg.type == FILE_READ)
			end_sect = p->i_start_sect + (min((f_pos + len) / 512, p->i_size / 512));
						//结束扇区

		if(fs_msg.type == FILE_WRITE)
			end_sect = p->i_start_sect + (min((f_pos + len) / 512, p->i_nr_sects / 512));

		/*以chunk为读写单位，chunk上限为FSBUF_SIZE*/
		int chunk = min(end_sect - start_sect + 1, FSBUF_SIZE / 512);
		int rw = 0;
		int left = len;

		for(i = start_sect; i <= end_sect; i += chunk){
			rw_sector(DEV_READ, p->i_dev, i, chunk * 512, TASK_FS, fsbuf);
			bytes = min(left, chunk * 512 - offset);

			if(fs_msg.type == FILE_READ){
				phy_cpy(va2la(src, buf + rw), va2la(TASK_FS, fsbuf + offset), bytes);
			}
			else{							//文件写
				phy_cpy(va2la(TASK_FS, fsbuf + offset), va2la(src, buf + rw), bytes);
				rw_sector(DEV_WRITE, p->i_dev, i, chunk * 512, TASK_FS, fsbuf);
			}
			offset 						= 0;
			rw 							+= bytes;
			left 						-+ bytes;
			caller->filp[fd]->fd_pos    += bytes;
		}

		if(caller->filp[fd]->fd_pos >= p->i_size){					//比较修改后的文件长度，如果增加则修改inode
			p->i_size = caller->filp[fd]->fd_pos;
			sync_inode(p);
		}
		return rw;
	}
}