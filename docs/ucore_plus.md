ucore+学习笔记

# arch

与硬件相关的代码大部分都被放置在`arch`目录下。主要包括了以下几个部分：

* bootloader（i386与ucore实现一致）
* OS入口（初始化部分）
* 设备驱动（这一部分的i386实现与ucore几乎完全一样）
    * 时钟
    * 终端显示（CGA/VGA）
    * 硬盘（IDE）
    * 键盘
    * PIC
* 中断使能
* 进程切换
* NUMA
* 中断处理