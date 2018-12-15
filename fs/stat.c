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

PUBLIC int fs_stat(){
	char pathname[MAX_PATH];
	char filename[MAX_NAME];
	int len = fs_msg.NAME_LEN;
	int src = fs_msg.source;
	struct inode * pin = 0;
	struct inode * dir_inode;

	phy_cpy(va2la(TASK_FS, pathname), va2la(src, fs_msg.PATHNAME), len);
	pathname[len] = 0;

	int inode_nr = search_file(pathname);
	if (inode_nr == 0) {	               					          //文件未找到
		printl("{FS} FS::do_stat():: search_file() returns "
		       "invalid inode: %s\n", pathname);
		return -1;
	}

	if (strip_path(filename, pathname, &dir_inode) != 0) {
		assert(0);
	}
	pin = get_inode(dir_inode->i_dev, inode_nr);

	int size = pin->i_size;

	put_inode(pin);

	struct status* s = (struct status*)va2la(src, fs_msg.STAT);

	s->size = size;

	return 0;
}