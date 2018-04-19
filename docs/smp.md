# RISC-V多核

* `mhartid`：当前所处的硬件线程（hart）的id（一定有一个hart的id为0）
*  `amo...`：原子操作，可以用于实现资源的互斥访问

## Inter-Processor Interrupts (IPI)

* 每个mode有一个自己的context

# pk/bbl

* 0号线程先进行初始化，其余线程用`wfi`等待0号线程的软件中断后再进行初始化。只有id在0到`MAX_HARTS-1`之间且未被mask掉的线程最终能够得到初始化。0号线程在初始化时，做的事情不只是初始化线程本身，还需要初始化系统整体的一些资源和环境。
* bootloader过程分两个阶段，先从M`mret`到S，然后再S里面加载ELF，做进一步设置，再`sret`到入口位置执行
* 0号之外的其他线程初始化之后就进入不断`wfi`的状态（等之后的IPI让它们干活？）