#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "hd.h"
#include "global.h"


PRIVATE void interrupt_wait(){
	MESSAGE msg;
	send_recv(RECEIVE ,INTERRUPT, &msg);
}

PRIVATE void init_hd(){
	u8* NR_DISK = (u8*) 0x475;						//从物理地址0x475处读取硬盘数量
	printl("%d disks found \n", *NR_DISK);

	put_irq_handler(HD_IRQ, hd_handler);			//设置硬盘中断
	enable_irq(EN_SLAVER);							//打开从片
	enable_irq(HD_IRQ);								//打开硬盘中断
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

PUBLIC void task_hd(){
	MESSAGE msg;

	init_hd();						//初始化硬盘

	while(1){
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;
		switch(msg.type){
			case DEV_OPEN:

				hd_identity(0);
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
}

PUBLIC void hd_handler(){
	int status = in_byte(ATA0_STATUS_CMD);				//读状态寄存器恢复硬盘中断
	info_task(TASK_HD);									//通知任务
}