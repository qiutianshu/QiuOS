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