# QiuOS

微内核
多进程，不可抢占，按优先级调度
进程间通信
MINIX文件系统

运行步骤：
1.安装bochs 2.6.9
2.提供了两种启动方式
     a、从a.img软盘映像启动
     b、从80m.img硬盘映像启动
3.修改bochsrc文件第22行
     boot:c     从硬盘启动
     boot:a     从软盘启动
4.当前目录下运行 bochs，启动后按Alt+F1～Alt+F3切换控制台
  当前提供的命令 ps：       打印所有进程
               bxtar:    解压缩文件
               echo：    字符串输出
               
1.make all 
2.copy boot/loader.bin to QiuOS/usr
3.tar loader.bin into cmd.tar
4.copy cmd.tar into QiuOS/80m.img
5.run QiuOS to extract all files
6.copy QiuOS/80m.img to /PROJ 
7.copy boot.bin into img
     run
