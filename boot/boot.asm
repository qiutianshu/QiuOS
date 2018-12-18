org 0x7c00

	jmp boot_start

STACK_BASE			equ		0x7c00
TRANS_SECT_NR		equ		2
SECT_BUF_SIZE		equ		TRANS_SECT_NR * 512
SUPER_BLK_SEG		equ		0x70
ROOT_BASE			equ		0xf000			;根设备起始扇区
LOADER_OFF			equ		0x100
LOADER_SEG			equ		0x9000

SB_ROOT_INODE			equ		7 * 4			;root_inode在超级块中的偏移
SB_INODE_SIZE			equ		8 * 4			;inode_size偏移
SB_NR_IMAP_SECTS		equ		3 * 4
SB_NR_SMAP_SECTS		equ		4 * 4
SB_INODE_ISIZE_OFF		equ		9 * 4
SB_INODE_START_OFF		equ		10 * 4
SB_DIR_ENT_FNAME_OFF	equ		13 * 4
SB_DIR_ENT_SIZE			equ		11 * 4
SB_DIR_ENT_INODE_OFF	equ		12 * 4

disk_addr_packet:
	db	0x10				;0 packet字节数
	db	0					;1 保留
	db	TRANS_SECT_NR		;2 要传输的块数
	db	0					;3 保留
	dw	0					;4 目的地址偏移
	dw	SUPER_BLK_SEG		;6 目的段
	dd 	0					;8 lba低32位
	dd  0					;12 lba高32位

err:
	mov dh, 3
	call disp
	jmp $

boot_start:
	mov ax, cs 
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, STACK_BASE

	call clear_screen

	mov dh, 0				;开机字符
	call disp

	mov dword [disk_addr_packet + 8], ROOT_BASE + 1		;读取超级块至0x700处
	call read_sector
	mov ax, SUPER_BLK_SEG
	mov fs, ax											;超级块段地址

	mov dword [disk_addr_packet + 4], LOADER_OFF		;loader加载的偏移
	mov dword [disk_addr_packet + 6], LOADER_SEG		;loader加载的段

	mov eax, [fs:SB_ROOT_INODE]							;根目录inode号
	call get_inode

	mov dword [disk_addr_packet + 8], eax				;读取根目录
	call read_sector									;注意，kernel.bin和loader必须位于根目录前面，因为此处只读取2个扇区

	mov si, LoaderFile
	push bx 											;inode缓冲区偏移，保存

.str_cmp:
	add bx, [fs:SB_DIR_ENT_FNAME_OFF]					;文件名在dir entry中的偏移
.1:
	lodsb												;ds:si -> al
	cmp al, byte [es:bx]
	jz .2
	jmp .different

.2:
	cmp al, 0											;是否结束
	jz .found
	inc bx
	jmp .1

.different:
	pop bx 												;弹出inode首地址
	add bx, [fs:SB_DIR_ENT_SIZE]						;下一个dir entry
	sub ecx, [fs:SB_DIR_ENT_SIZE]						;根目录大小自减
	jz .not_found

	mov dx, SECT_BUF_SIZE
	cmp bx, dx
	jge	.not_found										;超出范围

	push bx
	mov si, LoaderFile
	jmp .str_cmp

.not_found:  
	mov dh, 2
	call disp
	jmp $

.found:
	pop bx
	add bx, [fs:SB_DIR_ENT_INODE_OFF]
	mov eax, [es:bx]									;inode_nr of loader
	call get_inode
	mov dword [disk_addr_packet + 8], eax				;start sector of loader

load_loader:
	call read_sector									;ecx <- size of loader
	cmp ecx, SECT_BUF_SIZE
	jl .done
	sub ecx, SECT_BUF_SIZE
	add word [disk_addr_packet + 4], SECT_BUF_SIZE		;写入地址自加
	jc err 												;溢出
	add dword [disk_addr_packet + 8], TRANS_SECT_NR
	jmp load_loader

.done:
	mov dh, 4 
	call disp
	jmp LOADER_SEG:LOADER_OFF
	jmp $

;-------------------------------------------------
;字符串
;-------------------------------------------------
LoaderFile				db		"loader.bin",0			;加载器名称
MessageLength			equ		9
BootMessage				db		"WELCOME.."
Message1				db		"No Loader"
Message2				db		"Error    "
Message3				db      "Booting  "


;-------------------------------------------------
;显示字符串，dh对应于字符串序号
;-------------------------------------------------
disp:
	mov ax, MessageLength
	mul ah
	add ax, BootMessage
	mov bp, ax
	mov ax, ds
	mov es, ax
	mov cx, MessageLength			;字符串长度
	mov ax, 0x1301					
	mov bx, 0x7 					;页号0  黑底白字0x7
	mov dl, 0
	int 0x10
	ret

;-------------------------------------------------
;清屏
;-------------------------------------------------
clear_screen:
	mov ax, 0x600
	mov bx, 0x700 
	mov cx, 0
	mov dx, 0x184f
	int 0x10
	ret

;-------------------------------------------------
;读取扇区
;输入：指向disk_addr_packet的指针
;返回: es:bx指向缓冲区，ax:bx指向缓冲区
;-------------------------------------------------
read_sector:
	xor ebx, ebx
	mov ah, 0x42
	mov dl, 0x80
	mov si, disk_addr_packet
	int 0x13 

	mov ax, [disk_addr_packet + 6]
	mov es, ax
	mov bx, [disk_addr_packet + 4]
	ret

;-------------------------------------------------
;读取inode
;输入：eax-> inode编号
;返回：eax 		首扇区逻辑地址
;	  ecx 		inode->i_size
;	  es:ebx	指向inode缓冲区
;-------------------------------------------------
get_inode:
	dec eax                 					;inode_nr - 1
	mov bl, [fs:SB_INODE_SIZE]
	mul bl 										;eax = (inode_nr - 1) * SB_INODE_SIZE;;首字节在inode array中的偏移
	mov edx, SECT_BUF_SIZE
	sub edx, dword [fs:SB_INODE_SIZE]			;要get的inode必须在1kb的inode array之内，否则报错
	cmp eax, edx
	jg  err
	push eax

	mov ebx, [fs:SB_NR_IMAP_SECTS]
	mov edx, [fs:SB_NR_SMAP_SECTS]
	lea eax, [ebx + edx + ROOT_BASE + 2]
	mov dword [disk_addr_packet + 8], eax
	call read_sector

	pop eax

	mov edx, dword [fs:SB_INODE_ISIZE_OFF]
	add edx, ebx
	add edx, eax								;i_size
	mov ecx, [es:edx]							;根目录大小(字节)

	add ax, word [fs:SB_INODE_START_OFF]

	add bx, ax
	mov eax, [es:bx]
	add eax, ROOT_BASE							;文件首扇区
	ret

times 510 - ($ - $$) db 0
dw 0xaa55