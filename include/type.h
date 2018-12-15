#ifndef _QIUOS_TYPE_H_
#define _QIUOS_TYPE_H_

typedef unsigned long long    	u64;
typedef unsigned int			u32;
typedef unsigned short			u16;
typedef unsigned char			u8;
typedef void (*int_handler)();
typedef void (*task_f)();
typedef void (*irq_handler)(int irq);
typedef void* system_call;

/*tar文件头*/
struct posix_tar_header{
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124     8进制字符串*/   
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
};

/*消息结构*/
struct mess1 {
	int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};
struct mess2 {
	void* m2p1;
	void* m2p2;
	void* m2p3;
	void* m2p4;
};
struct mess3 {
	int	m3i1;
	int	m3i2;
	int	m3i3;
	int	m3i4;
	u64	m3l1;
	u64	m3l2;
	void*	m3p1;
	void*	m3p2;
};
typedef struct {
	int source;
	int type;
	union {
		struct mess1 m1;
		struct mess2 m2;
		struct mess3 m3;
	} u;
} MESSAGE;

#define	RETVAL		u.m3.m3i1
#define DEVICE		u.m3.m3i2		//次设备号
#define REQUEST		u.m3.m3i3
#define PROC_NR		u.m3.m3i4
#define POSITION	u.m3.m3l1		//IN SECTOR
#define COUNT		u.m3.m3l2		//读取字节数BYTES 
#define BUF			u.m3.m3p1
#define BUF_LEN		u.m3.m3i1

#define PATHNAME	u.m3.m3p2		//文件操作
#define FLAGS		u.m3.m3i1
#define NAME_LEN	u.m3.m3i2
#define FD			u.m3.m3i3

#define PID 		u.m3.m3i2
#define STATUS		u.m3.m3i3

#define STAT 		u.m3.m3p1


typedef struct dev_drv_map{
	int drv_pid;					//主设备号映射到驱动
};			

#endif