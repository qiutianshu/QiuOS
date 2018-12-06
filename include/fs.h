#define MAGIC_V1	0xaabbccdd

struct super_block{
	u32		magic;
	u32		nr_inodes;					//inode数量
	u32		nr_sects;					//扇区数
	u32		nr_imap_sects;				//inode map占用扇区数
	u32		nr_smap_sects;				//sector map占用扇区数
	u32		n_1st_sect;					//数据区第一个扇区号
	u32		nr_inode_sects;				//inode array占用扇区数
	u32		root_inode;					//根目录inode
	u32		inode_size;					//inode结构体大小
	u32		inode_isize_off;			//i_size在inode结构体中的偏移
	u32		inode_start_off;			//i_start_sect在inode结构体中的偏移
	u32		dir_ent_size;				//根目录中条目大小
	u32		dir_ent_inode_off;			//inode_nr在dir_entry中的偏移
	u32		dir_enr_fname_off;			//name在dir_entry中的偏移

	int 	sb_dev;						//超级块所在设备的设备号
};

#define SUPER_BLOCK_SIZE		56		//超级块在硬盘中的大小

struct inode{
	u32		i_mode;						//权限、模式
	u32		i_size;						//文件大小
	u32		i_start_sect;				//文件的第一个扇区
	u32		i_nr_sects;					//文件预留扇区
	u8		_unused[16];				//对齐

	int 	i_dev;						//属于哪个设备
	int 	i_cnt;						//多少个进程正在使用
	int 	i_num;						//本inode位置
};

#define INODE_SIZE 				32 		//inode结构在磁盘中的大小
#define MAX_FILENAME_LEN		12		//最大文件名长度
#define ROOT_INODE 				1		//根目录inode
#define	NR_DEFAULT_FILE_SECTS	2048 	//根目录预留扇区数

struct dir_entry{
	int 	inode_nr;					//文件起始扇区号
	char 	name[MAX_FILENAME_LEN];
};

typedef struct file_desc{				//文件描述符
	int 				fd_mode;
	int 				fd_pos;		
	struct inode* 		fd_inode;		//指向inode_table中元素
};

#define DIR_ENTRY_SIZE		sizeof(struct dir_entry)

/* INODE::i_mode (octal, lower 12 bits reserved) */
#define I_TYPE_MASK     0170000
#define I_REGULAR       0100000
#define I_BLOCK_SPECIAL 0060000
#define I_DIRECTORY     0040000
#define I_CHAR_SPECIAL  0020000
#define I_NAMED_PIPE	0010000

#define DEV_CHAR_TTY	TASK_TTY 

#define MAX_PATH		128				//路径最大长度
#define MAX_NAME 		12				//文件名最大长度
#define NR_FILES 		64				//最大文件打开数
#define NR_FILE_DESC	64				//文件描述符数量
#define NR_INODES 		64 				//inode_table中元素数量
#define NR_SUPER_BLOCK	8				//超级块数量

#define O_CREATE		1
#define O_RW 			2
