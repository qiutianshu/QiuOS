# QiuOS

宏内核
多进程，不可抢占，按优先级调度
进程间通信
EXT文件系统


1.make all 
2.copy boot/loader.bin to QiuOS/usr
3.tar loader.bin into cmd.tar
4.copy cmd.tar into QiuOS/80m.img
5.run QiuOS to extract all files
6.copy QiuOS/80m.img to /PROJ 
7.copy boot.bin into img
     run