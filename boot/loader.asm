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
wSectorNo			dw			0				;记录当前搜索扇区
wNumOfSector		dw			0
wNextDataBlock		dw			0
%include "fat12hdr.inc"

	
;显示字符串长度默认10
	Message1			db 			"Loading   "
	Message2			db 			"Ready!    "
	Message3			db 			"Scan   Mem"
	KernelFileName		db			"KERNEL  BIN"
	BootMessage			db			"Booting   "
	NotFound 			db 			"No Kernel!"


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
		mov dh,2
		push Message1
		call DispStrInRM
		add sp,2

		xor ah,ah			
		xor dl,dl	
		int 13h										;软驱复位

		mov byte [wSectorNo],SectorNoOfRootDir		;根目录扇区号
		mov word [wNumOfSector],14					;根目录扇区数
.1:
		cmp word [wNumOfSector],0
		jz .FILE_NOT_FOUND							;没有找到KERNEL.BIN

		dec word [wNumOfSector]
		mov ax,BaseOfKernelFile
		mov es,ax
		mov bx,OffsetOfKernelFile
		mov ax,[wSectorNo]
		mov cl,1									;读取1个根目录扇区到es:bx
		call ReadSector								;0x7c92

		mov si,KernelFileName						;ds:si->内核文件名
		mov di,OffsetOfKernelFile 					;es:di
		mov dl,0x10 								;每个扇区的条目数
.4:
		cmp dl,0									;0x7c9d
		je .NEXT_SECTOR
		mov cx,0xb		
		dec dl							
		cld
.3:
		cmp cx,0
		je .FILE_FOUND 								;找到文件
		lodsb										;ds:si->al
		cmp al,byte [es:di]
		jne .NEXT_ENTRY
		inc di
		dec cx
		jmp .3

.NEXT_ENTRY:
		and di,0xffe0								;回到本条目
		add di,0x20									;下一个条目
		mov si,KernelFileName	
		jmp .4

.NEXT_SECTOR:
		inc byte [wSectorNo]						;下一个扇区
		jmp .1

.FILE_NOT_FOUND:
		mov dh,3
		push NotFound
		call DispStrInRM
		add sp,2
		jmp $

.FILE_FOUND:
;		mov ax,14									;0x7ced
		and di,0xffe0
		add di,0x1a									;此条目对应的起始簇号
		mov cx,[es:di]								;0x7cf5
		mov word [wNextDataBlock],OffsetOfKernelFile
NEXT_DATA_BLOCK:
		mov ah,0xe 
		mov al,"."
		mov bl,0xf 
		int 10h

		push cx										;暂存扇区
		add cx,31
		mov ax,BaseOfKernelFile
		mov es,ax
		mov bx,[wNextDataBlock]
		mov ax,cx

		mov cl,1									;读取数据区第一个扇区
		call ReadSector								;0x7d17
		pop ax
		call SelectorNoOfFAT
		cmp ax,0xff0
		jnb LOAD_OK									;最后一个簇加载完毕
		mov cx,ax
		add word [wNextDataBlock],0x200
		jmp NEXT_DATA_BLOCK

LOAD_OK:
		call KillMotor								;关闭软驱马达
		mov dh,3
		push Message2
		call DispStrInRM
		add sp,2
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

;ax存放逻辑扇区号，cl存放读取扇区数，es:bx指向缓冲区  
;int 13h中断，ah=2，al=扇区数，ch=柱面号，cl=起始扇区号，dh=磁头号，dl=驱动器号，es:bx->数据区
ReadSector:
	push bp
	mov bp,sp
	sub sp,1								;局部空间存放扇区数
	mov byte [bp-1],cl 						;存放读取的扇区数
	push bx									;暂存缓冲区
	mov bl,[BPB_SECPerTrk]					;每磁道扇区数
	div bl
	inc ah
	mov cl,ah								;起始扇区号
	mov ch,al								
	shr ch,1								;磁道号
	mov dh,al
	and dh,1								;磁头号
	pop bx 									;es:bx指向数据区

.read:
	mov ah,2
	mov al,[bp-1]							;扇区数 
	mov dl,0
	int 13h
	jc .read 								;出现错误一直读

	add sp,1
	pop bp

	ret


;根据扇区号寻找FAT项的值，参数ax，返回值ax
SelectorNoOfFAT:
	push bp
	mov bp,sp
	sub sp,2
	push es
	push ax
	mov ax,BaseOfLoader
	sub ax,0x100
	mov es,ax
	pop ax

	xor dx,dx
	mov bx,3
	mul bx
	mov bx,2
	div bx 							;ax->偏移，dx->余数
	cmp dx,0
	je EVEN
	mov byte [bp-2],1					;奇
	jmp NEXT
EVEN:
	mov byte [bp-2],0					;偶
NEXT:
	xor dx,dx
	mov bx,512
	div bx 							;ax->相对于FAT表的扇区号，dx->扇区内的偏移
	push dx

	add ax,1 						;加上第一个FAT表的扇区号
	mov bx,0						;es:bx->(BaseOfLoader-0x100):0
	mov cl,2						;读2个扇区
	call ReadSector

	pop dx
	add bx,dx
	mov ax,[es:bx]
	cmp byte [bp-2],1
	jnz EVEN2
	shr ax,4
EVEN2:
	and ax,0xfff

	pop es
	add sp,2
	pop bp
	ret
;关闭软驱马达
KillMotor:
	push dx
	mov dx,0x03f2
	mov al,0
	out dx,al
	pop dx
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
