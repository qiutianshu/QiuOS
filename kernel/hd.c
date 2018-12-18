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

PRIVATE struct hd_info 	hdinfo[1];
PRIVATE char hdbuf[512];

PRIVATE void interrupt_wait(){
	MESSAGE msg;
	send_recv(RECEIVE ,INTERRUPT, &msg);
}

PRIVATE void init_hd(){
	u8* NR_DISK = (u8*) 0x475;						//从物理地址0x475处读取硬盘数量
	int i;
	printl("%d disks found \n", *NR_DISK);

	put_irq_handler(HD_IRQ, hd_handler);			//设置硬盘中断
	enable_irq(EN_SLAVER);							//打开从片
	enable_irq(HD_IRQ);								//打开硬盘中断

	for(i = 0; i < (sizeof(hdinfo)/sizeof(hdinfo[0])); i++){
		memset(&hdinfo[i], 0, sizeof(hdinfo[0]));
	}
}

PRIVATE int waitforhd(int mili_timeout){			//等待硬盘准备好
	int t = get_ticks();
	while((get_ticks() - t ) * 1000 / HZ <= mili_timeout){
		if(!(in_byte(ATA0_STATUS_CMD) & 0x80))
			return 1;
	}
	return 0;										//等待超时
}

PRIVATE void send_cmd_ata0(CMD* c){				//向ATA0发送命令
	if(!waitforhd(HD_TIMEOUT))						
		panic("Wait for hard disk timeout! ");

	out_byte(ATA0_STATUS_CTRL, 0);					//使能硬盘中断
	out_byte(ATA0_FEATURES,c->features);
	out_byte(ATA0_SECTOR_COUNT, c->sector_num);
	out_byte(ATA0_LBA_LOW,c->lba_low);
	out_byte(ATA0_LBA_MID,c->lba_mid);
	out_byte(ATA0_LBA_HIGH,c->lba_high);
	out_byte(ATA0_DEVICE, c->device);
	out_byte(ATA0_STATUS_CMD,c->cmd);
}

PRIVATE void hd_info_print(char* buf){
	int i,k;
	char tmp[64];
	memset(tmp, 0, sizeof(tmp));

	struct  hd_info_list{
		int idx;					//字偏移
		int len;					//字节长度
		char* name;
	}infolist[] = {{10, 20,  "HD   SN"},
					{27, 40, "HD MODE"}};

	for(i=0; i<sizeof(infolist)/sizeof(infolist[0]); i++){
		char* p = &buf[2 * infolist[i].idx];
		for(k=0; k<(infolist[i].len)/2; k++){
			tmp[2 * k + 1] = *p++;
			tmp[2 * k] = *p++;
		}
		tmp[2 * k] = 0;
		printl("%s : %s \n",infolist[i].name, tmp);
	}

	int capabilities = (int)*((u16*)(&buf[98]));
	printl("LBA supported: %s\n",(capabilities & 0x200)?"Yes":"No");

	int capacity = (int)(*(u16*)(&buf[122]) << 16) + (u16)*(u16*)(&buf[120]);
	printl("Capacity is : %d Mb\n", capacity / 2048);
}

PRIVATE void get_part_table(int drive, int sector, struct part_table_ent* entry){
	CMD cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.features = 0;
	cmd.sector_num = 1;
	cmd.lba_low = sector & 0xff;
	cmd.lba_mid = (sector >> 8) & 0xff;
	cmd.lba_high = (sector >> 16) & 0xff;
	cmd.device = SET_DEVICE_REG(((sector >> 24) & 0xf), drive, 1);		//LBA模式
	cmd.cmd = READ;
	send_cmd_ata0(&cmd);

	interrupt_wait();
	port_read(ATA0_DATA, hdbuf, 512);						//读取一个扇区
	char* ptr = hdbuf + 0x1be;
	memcpy(entry, ptr, sizeof(struct part_table_ent) * NR_PART_PER_DIV);
}

PRIVATE void partition(int device){
	int drive = DEV_TO_DIV(device);
	int i,j;
	int k = 1;									//指向下一个要写的part_table,primary[0]留给hd0
	struct hd_info* h = &hdinfo[drive];
	struct part_table_ent part_table[NR_SUB_PER_DIV];			//64

	get_part_table(drive, 0, part_table);		//获得mbr分区表

	for(i = 0; i < NR_PART_PER_DIV; i++){
		if(part_table[i].system_id == 0)
			continue;
		
		h->primary[i + 1].base = part_table[i].init_lba;				
		h->primary[i + 1].size = part_table[i].nr_sector;									
		h->primary[i + 1].flag = PRIMARY;

		if(part_table[i].system_id == 0x5){							//扩展分区
			h->primary[i + 1].flag = EXTEND;
			int ext_start_sector = part_table[i].init_lba;			//主分区表中扩展分区的起始位置
			int s = ext_start_sector;
			int nr_1st_sub = i * NR_SUB_PER_PART;					//扩展分区第一个逻辑分区次设备号0,16,32,48
			int nr_ext;
			for(j = 0; j < NR_SUB_PER_PART; j++){
				nr_ext = j + nr_1st_sub;							//当前逻辑分区次设备号
				get_part_table(drive, s, &part_table[4]);			//0-3装主分区表
				h->extend[nr_ext].base = s + part_table[4].init_lba;
				h->extend[nr_ext].size = part_table[4].nr_sector;
				h->extend[nr_ext].flag = LOGICAL;	
				s = ext_start_sector + part_table[5].init_lba;		//mbr指示的扩展分区起始地址起算

				if(part_table[5].system_id == 0)
					break;
			}
		}		
	}
}

PRIVATE void print_partition(struct hd_info* h){					//打印分区表
	int i;
	for(i = 0; i < NR_PRIME_PER_DIV; i++)
		printl("%shd%d:  base %d, size %d sectors %d MB           %s\n ",i == 0 ? "\n":"    ",
			    i, h->primary[i].base, 
			    h->primary[i].size, 
			    h->primary[i].size / 2048,
			    h->primary[i].flag == EXTEND ? "Extend" : "Primary");

	for(i = 0; i < NR_SUB_PER_DIV; i++){
		if(h->extend[i].size == 0)
			continue;
		printl("    hd%d%c: base %d, size %d sectors %d MB           Logical\n ",
				1 + i / 16,
				'a' + i % 16,
			 	h->extend[i].base,
			  	h->extend[i].size,
			  	h->extend[i].size / 2048);
	}
}

PRIVATE void hd_open(int device){
	int drive = DEV_TO_DIV(device);				//获得驱动号
	if(hdinfo[drive].open_cnt == 0){			//先判断是否打开，再自加1
		hdinfo[drive].open_cnt++;
		partition(drive * (NR_PRIME_PER_DIV));	
		print_partition(&hdinfo[drive]);
	}
}

PRIVATE void hd_close(int device){
	int drive = DEV_TO_DIV(device);
	hdinfo[drive].open_cnt--;
}

PRIVATE void hd_rw(MESSAGE* msg){
	int drive = DEV_TO_DIV(msg->DEVICE);
	int device = msg->DEVICE;
	int pos = msg->POSITION;					//相对扇区数
	pos += msg->DEVICE < NR_MAX_PRIME ? \
			hdinfo[drive].primary[device].base : \
			hdinfo[drive].extend[(msg->DEVICE - MINOR_hd1a) % NR_SUB_PER_DIV].base;
	CMD cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.features = 0;
	cmd.sector_num = (msg->COUNT + 512 -1) / 512;			//扇区数
	cmd.lba_low = pos & 0xff;
	cmd.lba_mid = (pos >> 8) & 0xff;
	cmd.lba_high = (pos >> 16) & 0xff;
	cmd.device = SET_DEVICE_REG((pos >> 24) & 0xf, drive, 1);
	cmd.cmd = (msg->type == DEV_READ) ? READ : WRITE;
	send_cmd_ata0(&cmd);

	int left = msg->COUNT;
	void* la = va2la(msg->PROC_NR, msg->BUF);
	while(left){
		int bytes = min(512,left);
		if(msg->type == DEV_READ){
			interrupt_wait();
			port_read(ATA0_DATA,hdbuf ,512);
			phy_cpy(la, (void*)va2la(TASK_HD, hdbuf), bytes);
		}
		else{
			int t = get_ticks();
			while(1){
				if((get_ticks() - t ) * 1000 / HZ <= HD_TIMEOUT){
					if(in_byte(ATA0_STATUS_CMD) & 0x08)
						break;
				}
				else
					panic("hd wait timeout *!");
			}
					
			port_write(ATA0_DATA, la, bytes);
			interrupt_wait();
		}
		left -= bytes;
		la += bytes;
	}
}

PRIVATE void hd_ioctl(MESSAGE* msg){							//返回设备的起始扇区和大小
	int drive = DEV_TO_DIV(msg->DEVICE);
	int device = msg->DEVICE;
	if(msg->REQUEST == DIOCTL_GET_GEO){
		void* dest = va2la(msg->PROC_NR, msg->BUF);
		void* src  = va2la(TASK_HD,
							msg->DEVICE <= NR_MAX_PRIME ?
							&hdinfo[drive].primary[device] :
							&hdinfo[drive].extend[(msg->DEVICE - MINOR_hd1a) % NR_SUB_PER_DIV]);
		phy_cpy(dest, src, sizeof(struct part_table_ent));
	}
	else 
		assert(0);
}

PUBLIC void task_hd(){
	MESSAGE msg;

	init_hd();						

	while(1){
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;
		switch(msg.type){
			case DEV_OPEN:
				hd_identity();
				hd_open(msg.DEVICE);
				break;
			case DEV_CLOSE:
				hd_close(msg.DEVICE);
				break;
			case DEV_READ:
			case DEV_WRITE:
				hd_rw(&msg);
				break;
			case DEV_IOCTL:
				hd_ioctl(&msg);
				break;
			default:
				printl("Unknown message type : %d\n",DEV_OPEN);
				spin("FS:main loop in kernel/hd.c line: 19");
				break;
		}
		send_recv(SEND, src, &msg);
	}
}

PUBLIC void hd_identity(int device){
	char buf[512];
	memset(buf, 0, sizeof(buf));
	CMD c;
	c.device = SET_DEVICE_REG(0, device, 0);			//HSC模式 ， 0号磁头
	c.cmd = IDENTIFY;
	send_cmd_ata0(&c);
	interrupt_wait();
	port_read(ATA0_DATA, buf, 512);						//读取256个字
	hd_info_print(buf);
	hdinfo[device].primary[0].base = 0;
	hdinfo[device].primary[0].size = (int)(*(u16*)(&buf[122]) << 16) + (u16)*(u16*)(&buf[120]);
}

PUBLIC void hd_handler(){
	int status = in_byte(ATA0_STATUS_CMD);				//读状态寄存器恢复硬盘中断
	info_task(TASK_HD);									//通知任务
}