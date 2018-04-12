# ucore+ kern-ucore/arch/i386 与ucore的区别

## 通用区别

- `defs.h`变为`types.h`
- `x86.h`变为`arch.h`
- `kmonitor.h`变为`monitor.h`
- `stdio.h`变为`kio.h`，同时所有`cprintf`变为`kprintf`
- 新增`mp.h`用于支持NUMA架构
- 各个子文件夹（`arch/i386/*`下）新增Makefile，语句为`obj-y = 各个c文件对应的.o文件`
- 新增`mod.h`，用于动态内核加载。



## configs

存放default config，包括工具链、默认调度算法、默认文件系统的设置。



##debug

#### kdebug.h & kdebug.c

新增DR6、DR7寄存器用于debug，新增加了debug函数的定义，用于ucore内部程序的debug.

#### (k)monitor.h & (k)monitor.c

新增加了一系列的指令：continue,step,break,watch,deldr,listdr,halt，主要用于ucore内部程序的debug。

#### assert.h

移除，放在了`kern-ucore/libs`中。



## driver

#### arch_sysconf.h

在sysconf中依赖的硬件相关的东西。在这里为空文件。

#### clock.h & clock.c

删除函数`long SYSTEM_READ_TIMER(void) {return ticks;}`。

#### ide.h & ide.c

一共有两个硬盘，对于channels（硬盘接口的I/O通道与Control通道）新增锁机制，保证同时只能读写一个硬盘。



## fs

放在了外面。TODO



## init

#### init.c

- 删除`lab1-challenge`相关的测试。
- 删除`grade_backtrace()`，即lab1用于测试`print_stackframe`时的函数
- 新增初始化：
  - `mp_init`，用于初始化NUMA架构
  - `debug_init`，用于初始化新增的debug寄存器
  - `pmm_init_ap`，初始化ap的CPU？
  - `sync_init`，调用`mbox_init`，初始化消息邮箱。似乎ucore+里支持进程之间的通信？
  - `mod_init`，发现是个空函数，可能他没写完module系列的使用？



## kmodule

#### mod.c

这是一个全新的模块。猜测可能和热加载有关？因为涉及到了重定向

- `apply_relocate`，将section重定向
- `apply_relocate_add`，没实现，调用会报错。



##libs

TODO



##mm 

#### buddy_pmm.h & buddy_pmm.c

将页分配算法更换为伙伴系统。从而删除了`default_pmm.h/c`和`kmalloc.h/c`。

#### memlayout.h

- 新增`PBASE`等于`KERNBASE`
- 将`struct Page`修改：
  - ref设置为原子量（其实就是含有volatile int的一个struct，要求编译器每次读取该值时都从内存中读取而不是从寄存器中）。（猜测原先ucore的实现是有bug的？）
  - 将swap机制通用化。（原先ucore只有lab3才有）
- 新增加了页表项的标志位和对应的操作语句。

#### mmu.h

- 新增了更多的多级页表，新增了页表项的操作函数（更加OOP）。

#### pmm.h & pmm.c

改动较多，TODO

（主要是因为更多级的页表）

#### swap.c

改动较多，TODO

####vmm.c

删除了许多通用的语句，放在了外面。



## NUMA



## process

TODO



## schedule

放在了外面

## sync

放在了外面



## syscall

增加了一些syscall



## trap

- 减小了TICK_NUM
- `idt_init`里把exception和iterupt的istrap也设置1，我认为是个bug
- 增加了对debug断点等的中断处理。