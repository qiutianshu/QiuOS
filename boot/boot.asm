	org 0x7c00
	jmp short LABEL_START
	nop

	;FAT12磁盘头
	BS_OEMName			db			"QiuOS   "
	BPB_BytsPerSec		dw			512
	BPB_SecPerClus		db			1
	BPB_RsvdSecCnt		dw			1
	BPB_NumFATs			db			2
	BPB_RootEntCnt		dw			224
	BPB_TolSec16		dw			2880
	BPB_Mdeia			db			0xf0
	BPB_FATSz16			dw			9
	BPB_SECPerTrk		dw			18
	BPB_NumHeads		dw			2
	BPB_HiddSec			dd			0
	BPB_TolSec32		dd			0
	BS_DrvNum			db			0
	BS_Reservedl		db			0
	BS_BootSig			db			0x29
	BS_VolID			dd			0
	BS_VolLab			db			"QiuOS_V0.01"
	BS_FileSysType		db			"FAT12   "

	wSectorNo			dw			0				;记录当前搜索扇区
	wNumOfSector		dw			0
	wNextDataBlock		dw			0

	TopOfStack			equ			0x7c00
	BaseOfLoader		equ			0x9000
	OffsetOfLoader		equ			0x100
	SectorNoOfRootDir	equ			19

	FileName			db			"LOADER  BIN"
	BootMessage			db			"Booting   "
	NotFound 			db 			"No Loader!"

	LABEL_START:
		mov ax,cs 
		mov ds,ax
		mov es,ax
		mov ss,ax
		mov sp,TopOfStack

		xor ah,ah			
		xor dl,dl	
		int 13h										;软驱复位

		mov byte [wSectorNo],SectorNoOfRootDir		;根目录扇区号
		mov word [wNumOfSector],14					;根目录扇区数
.1:
		cmp word [wNumOfSector],0
		jz .FILE_NOT_FOUND							;搜索完毕

		dec word [wNumOfSector]
		mov ax,BaseOfLoader
		mov es,ax
		mov bx,OffsetOfLoader
		mov ax,[wSectorNo]
		mov cl,1									;读取1个根目录扇区到es:bx
		call ReadSector								;0x7c92

		mov si,FileName								;ds:si->文件名
		mov di,OffsetOfLoader 						;es:di
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
		mov si,FileName	
		jmp .4

.NEXT_SECTOR:
		inc byte [wSectorNo]						;下一个扇区
		jmp .1

.FILE_NOT_FOUND:
		push NotFound
		call DispStr
		add sp,2
		jmp $

.FILE_FOUND:
		mov ax,0x600								;AH=6,AL=0    0x7cd6
		mov bx,0x700 								;BH=0X7,黑底白字
		mov cx,0									;左上角（0,0）
		mov dx,0x184f								;右下角（80,50）
		int 10h
		push BootMessage
		call DispStr
		add sp,2

;		mov ax,14									;0x7ced
		and di,0xffe0
		add di,0x1a									;此条目对应的起始簇号
		mov cx,[es:di]								;0x7cf5
		mov word [wNextDataBlock],OffsetOfLoader
NEXT_DATA_BLOCK:
		mov ah,0xe 
		mov al,"."
		mov bl,0xf 
		int 10h

		push cx										;暂存扇区
		add cx,31
;		add cx,17									;cx存放逻辑扇区号
		mov ax,BaseOfLoader
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
		jmp BaseOfLoader:OffsetOfLoader

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

;参数1=指向字符串的指针，长度默认10
DispStr:
	push bp
	push es
	mov bp,sp
	mov ax,ds
	mov es,ax
	mov cx,10							;长度
	mov ax,0x1301
	mov bx,0x0007
	mov dx,0							;行列
	mov bp,[bp+4]							;指向字符串的指针
	int 10h

	pop es
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




times 510-($-$$)		db        0
dw 0xaa55
