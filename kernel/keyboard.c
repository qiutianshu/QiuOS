#include "type.h"
#include "const.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "keyboard.h"
#include "keymap.h"
#include "global.h"

PRIVATE KB_INPUT kb_in;

PRIVATE int code_with_E0;
PRIVATE int shift_l;
PRIVATE int shift_r;
PRIVATE int ctrl_l;
PRIVATE int ctrl_r;
PRIVATE int alt_l;
PRIVATE int alt_r;
PRIVATE int colunm;
PRIVATE int caps_lock;
PRIVATE int scroll_lock;
PRIVATE int num_lock;

PRIVATE void kb_wait(){
	u8 kb_stat;
	do{
		kb_stat = in_byte(0x64);		//读取缓冲区状态
	}while(kb_stat & 0x2);
}

PRIVATE void kb_ack(){
	u8 kb_read;
	do{
		kb_read = in_byte(0x60);		//接收ack
	}while(kb_read != 0xfa);
}

PRIVATE void set_leds(){
	u8 led = (caps_lock << 2) | (num_lock << 1) | scroll_lock;		//led状态
	kb_wait();							//等待缓冲区空
	out_byte(0x60,0xed);				//向8048发送led设置命令
	kb_ack();
	kb_wait();
	out_byte(0x60,led);
	kb_ack();
}

PRIVATE u8 get_byte_from_buf(){
	u8 scan_code;
	while(kb_in.count <= 0){}			//等待下一个字节到来
	disable_int();						//关中断
	scan_code = *(kb_in.p_tail);
	kb_in.p_tail++;
	if(kb_in.p_tail == kb_in.buf + 128)
		kb_in.p_tail = kb_in.buf;		//循环读取
	kb_in.count--;
	enable_int();

	return scan_code;
}

PUBLIC void init_keyboard(){
	shift_l = shift_r = 0;
	ctrl_l = ctrl_r = 0;
	alt_l = alt_r = 0;
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;
	code_with_E0 = 0;

	caps_lock = 0;
	scroll_lock = 0;
	num_lock = 1;

	set_leds();							//设置键盘指示灯

	put_irq_handler(KEYBOARD_IRQ,keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}

PUBLIC void keyboard_handler(){
	u8 scan_code = in_byte(0x60);			//读取8042缓冲区
//	disp_int(scan_code);
	if(kb_in.count <= 128){					//缓冲区满则丢弃
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;
		if(kb_in.p_head == kb_in.buf + 128)
			kb_in.p_head = kb_in.buf;		//循环队列
		kb_in.count++;
	}
}

PUBLIC void keyboard_read(TTY* p_tty){
	u8 scan_code;
	char output[2] = {'\0','\0'};
	int make;				//0 make_code,1 break_code
	u32 key = 0;			//键值
	u32* keyrow;			//keymap的某一行
	int isPause = 1;
	int i;

	if(kb_in.count > 0){
		scan_code = get_byte_from_buf();
		if(scan_code == 0xe1){
			u32 Pause[]={0xe1,0x1d,0x45,0xe1,0x9d,0xc5};
			for(i=1; 1<6; i++){								//读取后面5个字节
				if(get_byte_from_buf() != Pause[i]){		
					isPause = 0;
					break;									//不符就退出
				}
			}
			if(isPause)
				key = PAUSEBREAK;
		}
		else if(scan_code == 0xe0){
			scan_code = get_byte_from_buf();
			if(scan_code == 0x2a){							//printscreen被按下
				if(get_byte_from_buf() == 0xe0)
					if(get_byte_from_buf() == 0x37){
						key = PRINTSCREEN;
						make = 1;
					}
			}
			if(scan_code == 0xb7){							//printscreen松开
				if(get_byte_from_buf() == 0xe0)
					if(get_byte_from_buf() == 0xaa){
						key = PRINTSCREEN;
						make = 0;
					}
			}
			/*不是PRINTSCREEN*/
			if(key == 0){
				code_with_E0 = 1;
			}
		}

		if((key != PAUSEBREAK) && (key != PRINTSCREEN)){
			make = (scan_code & 0x80 ? 0:1);					//判断make 还是 break
			keyrow = &keymap[(scan_code & 0x7f) * 3];
			colunm = 0;

			int caps = shift_r || shift_l;						//字母+shift = caps

			if(caps_lock){
				if(keyrow[0] >= 'a' && keyrow[0] <= 'z')
					caps = !caps;								//如果置位caps_locks，shift+字母 大写无效
			}
			if(caps)
				colunm = 1;
			if(code_with_E0){
				colunm = 2;
				code_with_E0 = 0;
			}
			key = keyrow[colunm];
			switch(key){
				case SHIFT_L:
					shift_l = make;
					break;
				case SHIFT_R:
					shift_r = make;
					break;
				case CTRL_L:
					ctrl_l = make;
					break;
				case CTRL_R:
					ctrl_r = make;
					break;
				case ALT_L:
					alt_l = make;
					break;
				case ALT_R:
					alt_r = make;
					break;
				case CAPS_LOCK:	
					if(make){
						caps_lock = !caps_lock;
						set_leds();
					}
					break;
				case SCROLL_LOCK:	
					if(make){
						scroll_lock = !scroll_lock;
						set_leds();
					}
					break;
				case NUM_LOCK:
					if(make){
						num_lock = !num_lock;
						set_leds();
					}
				default:
					break;
			}
			if(make){
				int pad = 0;
				if((key >= PAD_SLASH) && (key <= PAD_9)){
					pad = 1;
					switch(key){
						case PAD_SLASH:
							key = '/';
							break;
						case PAD_STAR:
							key = '*';
							break;
						case PAD_MINUS:
							key = '-';
							break;
						case PAD_PLUS:
							key = '+';
							break;
						case PAD_ENTER:
							key = ENTER;
							break;
						default:
							if(num_lock && (key >= PAD_0) && (key <= PAD_9))
								key = key - PAD_0 + '0';
							else if(num_lock && key == PAD_DOT)
								key = '.';
							else{
								switch(key){
									case PAD_HOME:
										key = HOME;
										break;
									case PAD_END:
										key = END;
										break;
									case PAD_PAGEUP:
										key = PAGEUP;
										break;
									case PAD_PAGEDOWN:
										key = PAGEDOWN;
										break;
									case PAD_INS:
									    key = INSERT;
									    break;
									case PAD_UP:
										key = UP;
										break;
									case PAD_DOWN:
										key = DOWN;
										break;
									case PAD_LEFT:
										key = LEFT;
										break;
									case PAD_RIGHT:
									 	key = RIGHT;
									 	break;
									case PAD_DOT:
									    key = DELETE;
									    break;
									default:
										break;
								}
							}
							break;
					}
				}

				key |=shift_l ? FLAG_SHIFT_L : 0;
				key |=shift_r ?	FLAG_SHIFT_R : 0;
				key |=ctrl_l  ? FLAG_CTRL_L  : 0;
				key |=ctrl_r  ? FLAG_CTRL_R  : 0;
				key |=alt_l   ? FLAG_ALT_L   : 0;
				key |=alt_r   ? FLAG_ALT_R   : 0;
				key |=pad     ? FLAG_PAD	 : 0;

				in_process(p_tty,key);
			}
		}
	}
}
