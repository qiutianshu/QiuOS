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

/*删除文件
*
*  删除成功返回0 失败返回-1
*/
PUBLIC int unlink(char* filepath){
//	printl("PATHNAME: %s\n ",filepath);
	MESSAGE msg;
	msg.type = FILE_DELETE;
	msg.PATHNAME = filepath;
	msg.NAME_LEN = strlen(filepath);

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}