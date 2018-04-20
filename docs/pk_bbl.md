RISC-V pk及BBL的学习笔记

# 区别和共性

* 共用了相同的M level trap处理机制以及一些基本的底层初始化
* `boot_loader`函数与`boot_other_hart`函数的实现不同
* pk接受参数，参数指定的是host上的文件。pk需要读取文件内容执行（目前还没研究怎么读取的文件）。BBL就是个bootloader，干的事情似乎就是简单地把控制权交出去（即利用`mret`跳到指定的位置并进入S mode）。

# 代码和注释中的一些缩写

* IPI：Inter-Processor Interrupt
* HLS：Hart-Local Storage
* FDT
* HTIF
* FPU：Floating-Point Unit
* PLIC：Platform-Level Interrupt Controller
* PMP：Physical-Memory Protection
* SUM: Supervisor User Memory
* FCSR: Feedback with Carry Shift Registers


# 中断服务

* pk工作在machine mode，处在supervisor mode的操作系统内核可以通过中断的方式与pk进行交互
* 把所有中断和异常都代理给了supervisor

