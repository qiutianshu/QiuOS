#include "stdio.h"
#include "system.h"

//struct posix_tar_header{
//	char name[100];		/*   0 */
//	char mode[8];		/* 100 */
//	char uid[8];		/* 108 */
//	char gid[8];		/* 116 */
//	char size[12];		/* 124     8进制字符串*/   
//	char mtime[12];		/* 136 */
//	char chksum[8];		/* 148 */
//	char typeflag;		/* 156 */
//	char linkname[100];	/* 157 */
//	char magic[6];		/* 257 */
//	char version[2];	/* 263 */
//	char uname[32];		/* 265 */
//	char gname[32];		/* 297 */
//	char devmajor[8];	/* 329 */
//	char devminor[8];	/* 337 */
//	char prefix[155];	/* 345 */
//};

int main(int argc, char const *argv[])
{
	if(argc == 1){
		printf("Usage:\n");
		printf("Try bxtar filename\n");
		return 0;
	}
/*	spin("cccccccccc");
	printf("extract file: %s\n", argv[1]);
	struct posix_tar_header* phd;
	char buf[8192];
	int fd_out;
	int size;												//压缩包内每个文件的大小										
	int fd = open(argv[1], O_RW);
	assert(fd != -1);
	while(1){
		read(fd, buf, 512);									//读取tar文件头
		if(buf[0] == 0){
			break;
		}

		size = 0;
		phd = (struct posix_tar_header*)buf;
		fd_out = open(phd->name, O_CREATE | O_RW | O_TRUNC);

		if(fd_out == -1){
			printf("failed to extract %s\n aborted", phd->name);
			close(fd);
			return;
		}		

		assert(fd_out != -1);
		char* p = phd->size;
		while(*p)
			size = (size * 8) + (*p++ - '0');

		printf(" %s(%d)\n", phd->name, size);
		while(size){
			int bytes = min(size, 8192);
			read(fd, buf, ((bytes - 1) / 512 + 1) * 512);	//读取整个扇区(tar文件扇区对齐)
			write(fd_out, buf, bytes);
			size -= bytes;
		}
		close(fd_out);
	}

	close(fd);
	printf("extract done!\n");*/
	return 0;
}