	jmp short LABEL_START
	nop

	;FAT12磁盘头
	BS_OEMName			db			"QiuOS0.01"
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

	SectorNoOfRootDir	dw			19
	wSectorNo			dw			0				;记录当前搜索扇区
	wNumOfSector		dw			0

	TopOfStack			equ			0x7c00
	BaseOfLoader		equ			0x9000
	OffsetOfLoader		equ			0x100

	FileName			db			"loader.bin"
	BootMessage			db			"Booting"
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
		call ReadSector

		mov si,FileName								;ds:si->文件名
		mov di,OffsetOfLoader 						;es:di
		mov dl,0x10 								;每个扇区的条目数
.4:
		cmp dl,0
		je .NEXT_SECTOR
		mov cx,0xa		
		dec dl							
		cld
.3:
		cmp cx,0
		je .FILE_FOUND 								;找到文件
		lodsb										;ds:si->al
		cmp al,byte [es:di]
		jne .NEXT_ENTRY
		inc si
		inc di
		dec cx
		jmp .3

.NEXT_ENTRY:
		and di,0xffe0								;回到本条目
		add di,0x20									;下一个条目
		mov si,FileName	
		jmp .3

.NEXT_SECTOR:
		inc byte [wSectorNo]						;下一个扇区
		jmp .1

.FILE_NOT_FOUND:
		mov bp,NotFound
		mov ax,ds
		mov es,ax
		mov cx,10
		mov ax,0x1301
		mov bx,0x0007
		mov bl,0
		int 10h
		mov ax,0x4c00
		int 0x21

.FILE_FOUND:
		jmp $
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
