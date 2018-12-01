
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

#define IDENTIFY	0xec
/*                     lba  设备    模式*/
#define SET_DEVICE_REG(lba, device, mode)		(0xa0 | (mode  << 6) | (device << 4) | lba)			//设置DEVICE寄存器

