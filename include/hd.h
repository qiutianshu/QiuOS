
/*硬盘控制端口号*/
#define ATA0_DATA			0x1f0
#define	ATA0_FEATURES		0x1f1
#define	ATA0_SECTOR_COUNT 	0x1f2
#define	ATA0_LBA_LOW		0x1f3
#define	ATA0_LBA_MID		0x1f4
#define	ATA0_LBA_HIGH		0x1f5
#define	ATA0_DEVICE			0x1f6
#define	ATA0_STATUS_CMD		0x1f7			//主控制器状态寄存器
#define ATA0_STATUS_CTRL	0x3f6

#define HD_TIMEOUT			10000

typedef struct part_info{					//分区信息   起始扇区，大小
	u32		base;
	u32		size;
	u8		flag;
};

typedef struct command{
	u8 data;
	u8 features;
	u8 sector_num;
	u8 lba_low;
	u8 lba_mid;
	u8 lba_high;
	u8 device;
	u8 cmd;

	u8 ctrl;
}CMD;

typedef struct part_table_ent{			//分区表条目结构
	u8  status;
	u8	init_head;
	u8	init_sector;
	u8	init_cylinder;
	u8	system_id;
	u8	end_header;
	u8	end_sector;
	u8	end_cylinder;
	u32	init_lba;
	u32	nr_sector;
};

/*                     lba  设备    模式*/
#define SET_DEVICE_REG(lba, device, mode)		(0xa0 | (mode  << 6) | (device << 4) | lba)			//设置DEVICE寄存器

#define NR_MAX_DIV			2										//主设备上最大硬盘数
#define NR_PART_PER_DIV		4										//mbr分区表元素个数
#define	NR_PRIME_PER_DIV	(NR_PART_PER_DIV + 1)					//每个硬盘主分区数量(算上设备本身)
#define NR_SUB_PER_PART		16								        //每个扩展分区最大逻辑分区数
#define NR_SUB_PER_DIV		(NR_SUB_PER_PART * NR_PART_PER_DIV)		
#define NR_MAX_PRIME		(NR_MAX_DIV * NR_PRIME_PER_DIV -1)		//主设备最大主分区数

#define MINOR_hd1a			0x10 
#define MINOR_hd2a			MINOR_hd1a + NR_SUB_PER_PART			//根设备的次设备号

#define MAKE_DEVICE(a , b)	((a << 8) | b)							//由主次设备号计算设备号
#define MAJOR(a)			((a >> 8) & 0xff)						//由设备号计算主设备号
#define MINOR(a)			(a & 0xff)								//由设备号计算次设备号
#define	DEV_TO_DIV(a)		(a <= NR_MAX_PRIME ? \
								a / NR_PRIME_PER_DIV \
							: (a - MINOR_hd1a) / NR_SUB_PER_DIV)	//由设备号计算属于哪个驱动器	

#define ROOT_DEV			MAKE_DEVICE(TASK_HD, MINOR_hd2a)		//计算根设备号

/*ata控制器命令*/
#define READ		0x20
#define WRITE		0x30
#define IDENTIFY	0xec

#define PRIMARY     1
#define EXTEND 		2
#define	LOGICAL		3

typedef struct hd_info{
	int 				open_cnt;					//打开数
	struct part_info	primary[NR_PRIME_PER_DIV];	//主分区信息
	struct part_info	extend[NR_SUB_PER_DIV];	//扩展分区信息
};


