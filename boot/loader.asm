	org 0x100
	jmp LABEL_START

%include "pm.inc"
	BaseOfStack			equ		0x100
	BaseOfLoader		equ		0x9000
	OffsetOfLoader		equ		0x100
	BaseOfKernelFile	equ		0x8000
	OffsetOfKernelFile	equ		0
	SectorNoOfRootDir	equ		19
	PageDirBase			equ		100000h		; 页目录开始地址: 1M
	PageTblBase			equ		101000h		; 页表开始地址:   1M + 4K

	TRANS_SECT_NR		equ		2
	SECT_BUF_SIZE		equ		TRANS_SECT_NR * 512
	KERNEL_FILE_SEG		equ		0x8000
	KERNEL_FILE_OFF		equ		0
	ROOT_BASE			equ		0xf000			;根设备起始扇区

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
	dw	KERNEL_FILE_OFF		;4 内核加载地址偏移
	dw	KERNEL_FILE_SEG		;6 内核加载段
	dd 	0					;8 lba低32位
	dd  0					;12 lba高32位



;GDT
;								段基址			段界限					段属性
GDT:		Descriptor       	 0,             0,                      0					;空描述符
VIDEO:		Descriptor	  		 0xb8000,		0xffff,		 			0x92|0x60			;显存
FLAT_RW:	Descriptor			 0,				0xfffff,				0x92 | 0x4000|0x8000;读写数据段
FLAT_C:		Descriptor			 0,				0xfffff,				0x9a | 0x4000|0x8000;读写执行代码段

GdtPtr		dw	$-GDT-1																		;GDT界限
			dd	0x90000+GDT														;GDT基地址(16位模式下低20位有效)


SelectorVideo		equ	 VIDEO	-	GDT 
SelectorFlatRw		equ	 FLAT_RW - GDT
SelectorFlatC		equ	 FLAT_C - GDT


;-------------------------------------------------------------------------------------------------------------------------------
;磁盘有关数据
;-------------------------------------------------------------------------------------------------------------------------------
;wSectorNo			dw			0				;记录当前搜索扇区
;wNumOfSector		dw			0
;wNextDataBlock		dw			0


	
;显示字符串长度默认10
	MessageLength		equ 		10
	Message1			db 			"Loading   "
	Message2			db 			"Ready!    "
	Message3			db 			"Scan   Mem"
	KernelFileName		db			"kernel.bin"
	BootMessage			db			"Booting   "
	NotFound 			db 			"No Kernel!"
	FOUND 				db 			"Success!  "
	ERROR 				db 			"Error!    "

err:
	mov dh, 7
	call disp
	jmp $

LABEL_START:
		mov ax,cs 
		mov ds,ax
		mov es,ax
		mov ss,ax
		mov gs,ax
		mov sp,BaseOfStack

;扫描内存信息
;------------------------------------------------------------------------------------------------------------------------
		mov dh,1
		push Message3
		call DispStrInRM
		add sp,2

		mov ebx,0
		mov di,_MemChkBuf
.loop:
		mov ecx,20
		mov edx,0x534d4150
		mov eax,0xe820
		int 15h
		jc CHACK_FAIL						;CF=1存在错误
		add di,20
		inc dword [_dwMCRNumber]
		cmp ebx,0
		jne .loop
		jmp CHECK_OK						;最后一个地址范围描述符
CHACK_FAIL:
		mov dword [_dwMCRNumber],0
CHECK_OK:
;加载内核
;-------------------------------------------------------------------------------------------------------------------------
	mov eax, [fs:SB_ROOT_INODE]							;根目录inode号(借用的boot中的fs)
	call get_inode

	mov dword [disk_addr_packet + 8], eax				;读取根目录
	call read_sector									;注意，kernel.bin和loader必须位于根目录前面，因为此处只读取2个扇区

	mov si, KernelFileName
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
	mov si, KernelFileName
	jmp .str_cmp

.not_found:  
	mov dh, 5
	call disp
	jmp $

.found:
	pop bx
	add bx, [fs:SB_DIR_ENT_INODE_OFF]
	mov eax, [es:bx]									;inode_nr of Kernel
	call get_inode
	mov dword [disk_addr_packet + 8], eax				;start sector of Kernel

load_kernel:
	call read_sector									;ecx <- size of loader
	cmp ecx, SECT_BUF_SIZE
	jl .done
	sub ecx, SECT_BUF_SIZE
	add word [disk_addr_packet + 4], SECT_BUF_SIZE		;写入地址自加
	jc .1 												;kernel超过64kb
	jmp .2

.1:
	add	word  [disk_addr_packet + 6], 1000h

.2:
	add dword [disk_addr_packet + 8], TRANS_SECT_NR
	jmp load_kernel

.done:
	mov dh, 6
	call disp

;进入保护模式 
;--------------------------------------------------------------------------------------------------------------------
;加载GDT
		lgdt [GdtPtr]

;关中断
		cli

;打开A20地址线
		in al,92h
		or al,00000010b
		out 92h,al

;设置CR0寄存器
		mov eax,cr0
		or  eax,1
		mov cr0,eax

		jmp dword SelectorFlatC:(0x90000 + LABEL_PM)			;跳转到32位实模式0x902d0

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
;-------------------------------------------------
;显示字符串，dh对应于字符串序号
;-------------------------------------------------
disp:
	mov ax, MessageLength
	mul ah
	add ax, Message1
	mov bp, ax
	mov ax, ds
	mov es, ax
	mov cx, MessageLength			;字符串长度
	mov ax, 0x1301					
	mov bx, 0x7 					;页号0  黑底白字0x7
	mov dl, 0
	int 0x10
	ret

;参数1=指向字符串的指针，长度默认10,dh指定行号
DispStrInRM:
	push bp
	push es
	mov bp,sp
	mov ax,ds
	mov es,ax
	mov cx,10							;长度
	mov ax,0x1301
	mov bx,0x0007
	mov dl,0							
	mov bp,[bp+6]							;指向字符串的指针
	int 10h

	pop es
	pop bp
	ret


[SECTION .s32]
ALIGN 32
[BITS 32]
LABEL_PM:
	mov ax,SelectorFlatRw				;整个0-4GB空间都是代码段、数据段、栈段
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov esp,TopOfStack
	mov ax,SelectorVideo
	mov gs,ax

	push szMemChkTitle
	call DispStr 				;0x904df
	add esp,4
	call DispMemSIze

	call SetupPaging			;设置分页 0x903c7
;映射内核代码
	call InitKernel

	mov eax,[dwMemSize]
	mov [Memsize],eax

;向内核转移控制权
	jmp SelectorFlatC:0x30400
	


SetupPaging:
	xor edx,edx
	mov eax,[dwMemSize]
	mov ebx,0x400000
	div ebx
	mov ecx,eax
	test ebx,ebx
	jz .1
	inc ecx
.1:
;	初始化页目录
	mov [PageTableNumber],ecx				;页表个数
	mov ax,SelectorFlatRw
	mov es,ax
	mov edi,PageDirBase
	xor eax,eax
	mov eax,PageTblBase
	or  eax,0x7
.2:
	stosd								
	add eax,4096
	loop .2
;初始化页表
	mov eax,[PageTableNumber]
	mov ebx,1024
	mul ebx
	mov ecx,eax
	mov edi,PageTblBase
	xor eax,eax
	or eax,0x7
.3:
	stosd
	add eax,4096
	loop .3

	mov eax,PageDirBase
	mov cr3,eax
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
.4:
	ret


DispMemSIze:										;显示内存段信息
	push esi
	push ecx
	mov esi,MemChkBuf
	mov ecx,[dwMCRNumber]
.1:
	mov edx,5
	mov edi,ARDStruct
.2:
	push dword [esi]
	call DispInt
	pop eax
	mov dword [edi],eax
	add edi,4
	add esi,4
	dec edx
	jnz .2
	cmp dword [dwType],1
	jne .3
	mov eax,[dwBaseAddrLow]
	add eax,[dwLengthLow]
	cmp eax,[dwMemSize]
	jb .1
	mov [dwMemSize],eax
.3:
	call DispReturn
	dec ecx
	jnz .1
	push szRAMSize
	call DispStr
	add esp,4
	push dword [dwMemSize]
	call DispInt
	add esp,4

	pop ecx
	pop esi
	ret

InitKernel:
	xor esi,esi
	mov cx,[0x80000 + 0x2c]				;PHT条目
	movzx ecx,cx
	mov esi,[0x80000 + 0x1c]			;e_phnum
	add esi,0x80000						;PHT 
.Begin:
	mov eax,[esi]
	cmp eax,0
	jz .Next_Table						;不是可加载段
	push dword [esi+0x10]				;size
	mov eax,[esi+0x4]					;该段在文件中的偏移
	add eax,0x80000
	push eax							;源地址
	push dword [esi+0x8]				;目的地址
	call MemCpy
	add esp,12
.Next_Table:
	add esi,0x20
	dec ecx
	jnz .Begin

	ret

DispInt:											;16进制显示一个整形数
	push eax
	mov eax,[esp+8]
	shr eax,24
	call DispAl

	mov eax,[esp+8]
	shr eax,16
	call DispAl

	mov eax,[esp+8]
	shr eax,8
	call DispAl

	mov eax,[esp+8]
	call DispAl

	push edi
	mov ah,0xa
	mov al,"h"
	mov edi,[dwDispPos]
	mov [gs:edi],ax
	add edi,4
	mov [dwDispPos],edi

	pop edi
	pop eax

	ret


DispAl:												;显示AL中的内容
	push eax
	push ecx
	push edx
	push edi

	mov edi,[dwDispPos]
	mov ecx,2
	mov ah,0xa
	mov dl,al										;al的副本
	shr al,4
.3:
	and al,01111b
	cmp al,9
	ja .1
	add al,"0"
	jmp .2
.1:
	sub al,0xa
	add al,"A"
.2:
	mov [gs:edi],ax
	add edi,2
	dec ecx
	jz .4
	mov al,dl
	jmp .3
.4:
	mov [dwDispPos],edi
	pop edi
	pop edx
	pop ecx
	pop eax
	ret
	
DispStr:								;显示字符串
	push ebx
	push edi
	push esi

	mov esi,[esp+0x10]					;指向字符串地址
	mov edi,[dwDispPos]
	mov ah,0x0a

	cld									;正向传输
.3:
	lodsb
	test al,al
	jz .1
	cmp al,0xa							;回车
	jne .2
	push eax
	mov eax,edi
	mov bl,160
	div bl
	and eax,0xff						;取行数
	inc eax
	mov bl,160
	mul bl
	mov edi,eax
	pop eax
	jmp .3
.2:
	mov [gs:edi],ax
	add edi,2
	jmp .3
.1:
	mov [dwDispPos],edi
	pop esi
	pop edi
	pop ebx
	ret

DispReturn:								;换行
	push szReturn
	call DispStr
	add esp,4
	ret


MemCpy:		
	push ebp
	mov ebp,esp
	push edi
	push esi
	push ecx

	mov edi,[ebp+8]		;目的地址
	mov esi,[ebp+12]	;源地址
	mov ecx,[ebp+16]	;长度

.1:
	cmp ecx,0
	jz .2
	mov al,[ds:esi]
	mov byte [es:edi],al
	inc edi
	inc esi
	dec ecx
	jmp .1
.2:
	mov eax,[ebp+8]		;返回目的地址
	pop ecx
	pop esi
	pop edi
	pop ebp

	ret

[SECTION .data]
ALIGN 32

SEG_DATA:
;内存有关数据
;------------------------------------------------------------------------------------------------------------------------------
;实模式下使用
_szPMMessage:			db	"In Protect Mode now.", 0Ah, 0Ah, 0								;进入保护模式后显示此字符串
_szMemChkTitle:			db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0			; 进入保护模式后显示此字符串
_szRAMSize				db	"RAM size:", 0
_szReturn				db	0Ah, 0																;换行符

_wSPValueInRealMode		dw  0
_dwMCRNumber:			dd	0																	;内存块数量
_dwDispPos:				dd	(80 * 6 + 0) * 2	; 屏幕第 6 行, 第 0 列。							;指向下一个显示位置
_dwMemSize:				dd	0		
_PageTableNumber		dd  0															
_ARDStruct:																					;返回结果的数据结构
	_dwBaseAddrLow:		dd	0																
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:			dd	0

_MemChkBuf:	times	256	db	0																;存放int 15h返回结果

;保护模式下使用
szPMMessage 		equ		0x90000 + _szPMMessage
szMemChkTitle		equ		0x90000 + _szMemChkTitle 
szRAMSize 			equ		0x90000 + _szRAMSize 
szReturn 			equ		0x90000 + _szReturn 
dwMCRNumber 		equ		0x90000+ _dwMCRNumber 
dwDispPos 			equ     0x90000+ _dwDispPos 
dwMemSize 			equ		0x90000+ _dwMemSize 
ARDStruct 			equ		0x90000+ _ARDStruct 
	dwBaseAddrLow 	equ		0x90000+ _dwBaseAddrLow 
	dwBaseAddrHigh  equ		0x90000+ _dwBaseAddrHigh 
	dwLengthLow 	equ		0x90000+ _dwLengthLow 
	dwLengthHigh 	equ		0x90000+ _dwLengthHigh 
	dwType 			equ		0x90000+ _dwType 
MemChkBuf 			equ		0x90000+ _MemChkBuf
PageTableNumber     equ     0x90000+ _PageTableNumber 
Memsize 			equ		0x900


StackSpace:	times	1024	db	1
TopOfStack	equ	0x90400+ StackSpace				;栈顶

DataLen				equ		$ - SEG_DATA
