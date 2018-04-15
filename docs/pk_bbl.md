RISC-V pk及BBL的学习笔记

# 代码和注释中的一些缩写

* IPI：Inter-Processor Interrupt
* HLS：Hart-Local Storage
* FDT
* HTIF
* FPU：Floating-Point Unit
* PLIC：Platform-Level Interrupt Controller
* PMP：Physical-Memory Protection
* SUM: Supervisor User Memory


# 中断服务

* pk工作在machine mode，处在supervisor mode的操作系统内核可以通过中断的方式与pk进行交互
* 把所有中断和异常都代理给了supervisor

