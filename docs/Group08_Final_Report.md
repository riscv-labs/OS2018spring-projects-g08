# Group08 大实验报告

##### 成员

计52 路橙 2015010137

计52 于志竟成 2015011275 

## 实验目标描述

实现ucore+向**64位RISC-V（spec为1.10）**的移植，借鉴xv6开启简单的**SMP**，并完成**LKM**（loadable kernel module）的RISC-V移植，最后进行多核性能的优化。

具体而言，我们的目标分为如下几点：

1. 将ucore for RISC-V 64的前人移植工作跑通。（已完成）
2. 借鉴前人在x86上的SMP开启工作，将ucore在RSIC-V 64上开启SMP（同时可借鉴BBL）。（已完成，测例基本全部通过）
3. 学习ucore+和ucore的差别。（已完成，文档在git仓库）
4. 学习RISC-V 64的特性。（已完成，文档在git仓库）
5. 针对ucore移植的日志，将ucore+移植到RISC-V 64上。（已完成，移植文档及代码在git仓库）
6. 将ucore+的LKM移植到RISC-V 64。（已完成）
7. 研究Linux的HAL设计，看如何对ucore+进行改进。（未完成）
8. 对ucore+在RISC-V上开启SMP。（已完成）
9. 将LKM移植到开启SMP后的ucore+上（RISC-V64）。（已完成）
10. 尝试将sv6的一些特性移植到ucore+，改进ucore+的并行性能。（未完成）
11. 将32bit与64bit的RISC-V代码合并。（未完成）



## 已有相关工作介绍

### ucore+的文档完善

见[ucore+ for RISC-V 64 单核移植文档](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore%2B_rv64_porting.md)。

由于原有ucore+对代码体系的介绍十分匮乏，且没有一个tutorial来引导如何新增一个arch移植。因此，我们组首先调研了ucore+，并在移植risc-v的过程中更新文档。

具体而言，文档分为`Makefile体系介绍`以及`risc-v 64bit单核移植过程`。由于ucore+的makefile体系十分复杂，因此这样的一个文档对后人的大实验开展十分有利。

我们最终形成的文档既是我们的移植文档，也是**ucore+的代码结构文档**，更可以**为后人提供arch移植的参考**。



### ucore+的bug修复

在移植risc-v以及开启smp的过程中，我们修复了一些原有ucore+的bug。列举如下：

- `vmm.c`中`do_pgfault`函数，在share memory的情况下，实现lazy分配。因此do_pgfault会为share memory的vmm分配一个物理页，最终将该物理页插入到页表中。而实现中有一句话：

  ```c
  page_insert(mm->pgdir, pa2page(*sh_ptep), addr, perm);
  ```

  可以看到，这里`sh_ptep`是一个页表项，得到虚拟地址时不可以用`pa2page`，而应该用`pte2page`。

  因此，代码需要修改为：

  ```
  page_insert(mm->pgdir, pte2page(*sh_ptep), addr, perm);
  ```

  而原有的x86之所以可以正常运行，是因为x86的页表项长度与物理地址长度相同（比如i386都为4字节），因此页表项与物理地址的前20位完全相同且对齐。故两个函数返回结果相同。

  但在risc-v中，页表项的长度与物理地址的长度不同，因此两个函数的实现不同，不可以替换。

  

  页表错误会导致bug的现象十分奇怪，往往很难调试。而这个bug从现象到原理的分析更是与计原的学习密不		可分。具体而言，这个bug的表现形式为：

  - 用户程序申请share_memory失败，跟踪后发现经过系统调用后得到的share memory的地址是合法的，说明用户向内核成功申请了一块虚拟地址。而后面对于该地址的检查代码出现了错误。

  - 继续跟踪，发现在检查的过程中，某个变量的行为很奇怪：

    - 相邻两次连续cprintf该变量的值，第一次和第二次的结果不同。经过分析，因为第一次直接把该变量在寄存器的值当做了cprintf的参数（编译器的优化），而第二次因为cprintf修改了寄存器，因此只能从内存读取，故两次结果不同。若把该变量定义为volitile，则两次结果一直且全都是0.
    - 向该内存中写数据后再读取，无论写什么，依然是0。

    由于risc-v采取的是RISC架构，根据计原对MIPS的学习，RISC架构32位的低2GB物理空间用来硬件地址的直接映射，因此怀疑该变量在页表中被映射到了一块低2GB的物理空间。经过检查，发现果然是这样。因此bug定位到了页表的问题，也就很快发现了bug所在位置。

- `sched.c`中多级队列的设定与语义相悖。ucore+的框架很好地支持了per cpu的runqueue，只需要改动很少代码即可支持。但在`sched.c`中却把所有的runqueue拼接到了一起，十分匪夷所思，因此我们修改了`process`文件夹下很多关于进程调度的代码。

- 在i386和amd64架构下的原子操作的实现不正确。具体而言：

  - i386没有实现原子操作。
  - amd64的原子操作没有加前缀"LOCK"。

  因此，两个体系下对并行的支持都不是很好。



### RISC-V 64 bit 在ucore+上的移植

我们成功将RISC-V 64 bit在ucore+上移植，单核情况下全部测例都可以通过。

#### 与前人工作的对比

石振兴已经成功在ucore上移植risc-v 64bit，我们的工作主要参考了他的实现、文档以及张蔚在risc-v 32bit移植的文档。

而我们的移植在前人的工作上改动了很多，也修复了很多前人的bug。具体列举如下：

- 前人的页表为2级页表，与32位相同，利用了RISC-V的大页机制。而我们将页表**全部设为3级页表**，且可以和32位的2级页表**共用同一套代码（只有一些宏定义不同）**，从而为之后32位与64位代码合并提供了很大的便利。
- 前人的虚拟内存仍为32位，内核虚拟地址与物理地址是恒等映射。我们将**虚拟地址修改为64位**，虚拟地址布局也进行了相应的修改，可以轻松支持**128GB**甚至更大的物理内存空间，真正发挥了64位的优点。此外，我们的**内核虚拟地址到物理地址的映射不是恒等映射**，满足了OS通用性的需求。
- 前人对`none leaf page table`设置了`PTE_U`，与spec违背。我们将其删除。
- 前人对`print_pgdir`和`get_pgtable_items`的实现是错误的，没有考虑高级页表直接作为`leaf page table`的情形。我们将其删除。
- 前人没有实现`spinlock`，且原子操作的实现很容易出现地址不对齐的情形，并且原子操作也没有考虑`fence`。我们将这些问题修复。
- 前人在`load_cr3`函数的实现有bug。在`load_cr3`前，必须要全部刷掉TLB，否则访问新的页表时，若TLB中存在该页表，则直接访问了TLB中的旧页表项。
- 前人在`copy_thread`时有bug，将tf减了4，这会导致栈不对齐等一系列问题，应当删去。
- 前人对`ramdisk`的实现是写死的，而我建立了虚实地址映射的机制，通过虚拟地址读写软盘，从而为后续增加文件系统以及交换区带来便利（共用一套机制）。
- 前人没有实现`print_stackframe`功能。我们参考Linux的实现，给出了详细的函数调用栈以及各个栈指针指向的地址。由于RISC-V与x86的实现原理不同，我们暂时还没有实现符号解析，函数调用栈中保存了各个函数代码的地址。可以通过`objdump`等方式查看ELF文件，在debug时也很方便。

#### 主要bug记录

##### Lab1

主要是makefile的问题，ucore+的makefile体系十分不同。可以参考我们的移植文档。

##### Lab2

- ucore+使用的多级页表是模板，因此需要在mmu.h中定义一个假的4级页表来模拟risc-v 64bit的3级页表（第4级和第3级相同）。这样的做法可以让之后risc-v 64bit与32bit的代码合并具有很大的便利。
- 卡在enable_paging出不来，最后发现是因为mp_init没有打开。因为ucore+的numa机制是通用的模板，需要适配这个。（并帮助group03修了这个bug）
- risc-v 64bit的多级页表，对于none-leaf page table而言，需要R、W、X位都为0（期中考试也有涉及），且参考risc-v 1.10手册可知D、A、U位也需要置零。因此需要修改ucore+下通用pmm.c的get_pte，加上risc-v硬件的判断。
- risc-v的leaf page table entry中，如果W为1，那么R也必须为1，否则CPU会报错。因此设置页表项的W、R的函数必须进行相应修改。（补充：后续设置vmm_flags时也需要相应修改）
- risc-v中页表项和物理地址的长度不同，因此设置页表项对应的物理地址时不能像x86那样简单地进行进行或操作，需要移位。

##### Lab3

- 移植swap的check函数时，需要注意risc-v的虚拟地址只有低48位有效，更高位是保留位，因此在建立第一个三级页表项下所有的页映射时，循环的条件修改为while (!((addr0 >> 39) & 0x1FFFFFF))，即只取三级页表对应的虚拟地址的位。 
- 移植swap的check函数时，需要把PTSIZE等按照x86 amd64的类型改成PUSIZE、PMSIZE等，即使用通用的多级页表机制，方便之后增加32 bit代码。
- 参考了ARM下关于ramdisk_file的makefile和ld文件，最终成功加入了ide驱动，并且可以在编译层面控制是否使用ramdisk。

##### Lab4 & Lab5

- init_new_context需要使用64bit的栈，因此需要考虑对齐的问题。比如，将所有bool变量的定义改为unsigned long long。否则会产生AMO address misaligned 错误。
- ucore+改了sys_exec的代码，因此传参需要进行调整。我的这条commit记录也帮助group03修复了这个bug。
- vmm.c中需要增加vm_flags的修改。具体而言，需要在有VM_WRITE时将perm设置为PTE_W|PTE_R（参考lab2移植的记录）。此外，由于risc-v中有PTE_X，而x86没有考虑这一点，因此在vm_flags含有VM_EXEC时，需要将perm的设置增加PTE_X。我的这条commit记录顺利地帮助group03跑出了sh。 

##### Lab6 & Lab7 & Lab8

几乎不需要修改。



### LKM在RISC-V 64bit上的移植

- SFAT文件系统已经可以作为一个LKM在ucore+ RISC-V 64上正常运行。



### RISC-V 64bit上SMP的开启

我们将RISC-V 64bit开启SMP的硬件相关工作全部完成，软件相关工作只剩下一个较明显的同步互斥相关的bug。

单核情况下，测例通过39/39.

多核情况下，测例通过38/39，`primer3`有时候无法通过。（注：`matrix`测例在核数大于等于4时无法通过，因为该测例会检查`pid = 4`的进程的输出。对于多核而言，核数大于等于4时，`pid=4`的进程为idle进程，不可能有输出）

#### 硬件相关的开启

- 内核栈需要每个CPU一个。因此需要修改`entry.S`

- 原子操作的实现使用了`gcc`的内置函数，更安全可靠。但RISC-V需要保证原子操作指令访问的数据的地址8字节对齐。

- 每个CPU需要保存一些额外的信息（比如CPU编号），通过结构体`struct cpu`定义。每个处理器的信息保存在各自的cpu结构体中，而结构体的指针由`tp`寄存器（thread pointer）保存。（该实现参考了Linux RISC-V 64）。由于`tp`寄存器只会在C++11中被用到，现在的ucore+在内核态和用户态都不会用到，因此目前的实现是安全可靠的。

- 时钟中断来临时，对`console`进行输出的操作仅由BSP（即0号CPU）完成。因为所有CPU共享了同一个输出缓存数组。

- 每个CPU需要保存的信息有：

  ```c
    unsigned int id;                    // cpu id
    struct context *scheduler;   // swtch() here to enter scheduler
    volatile unsigned int started;       // Has the CPU started?
    int ncli;                    // Depth of pushcli nesting.
    int intena;                  // Were interrupts enabled before pushcli?
    void* idle_kstack;      // kernel stack for idle process
    // Cpu-local storage variables; see below
    struct proc_struct *proc, *idle;           // The currently-running process.
    struct run_queue rqueue; // cpu specific run queue.
    size_t used_pages;      // size of used physical pages per cpu.
    list_entry_t page_struct_free_list; // free page list per cpu.
    spinlock_s rqueue_lock;             // lock for rqueue.
    struct __timer_list_t timer_list;   // timer_list per cpu.
    struct proc_struct* prev;           // before switch, the current proc.
  ```

#### 软件相关的实现

- `load_balance`操作。每个CPU的rqueue的大小应该尽量平均，因此需要对每个CPU的进程队列进行平衡。该操作在每次`schedule`时被调用。

- 在用户进程通过`malloc/brk`申请过多空间导致内存不足时的处理方式应为杀死进程而非panic。

- 共享资源需要合理上锁：`mbox`机制、`pmm`进行分配物理页时、`slab`进行内存分配时、`event`机制、`wait`机制。

- `kprintf`和`cprintf`需要上锁。

- 进程调度十分复杂，需要很精细的同步互斥控制。目前主要发现的问题有：

  - 多CPU下的`timer_list`是每个CPU一个的。需要保证进程wakeup后，仍然在原来的CPU上跑。由于存在跨CPU的唤醒操作（比如event机制下，某个进程可以wakeup任何满足条件的等待的进程），被唤醒的进程可能不在原来的CPU上，因此需要对`wakeup`函数进行额外的处理。

  - 当存在`load_balance`时，`schedule`函数的处理顺序必须为：dequeue(next) -> proc_run(next) -> switch(current, next) -> enqueue(current)。之所以enqueue(current)要放在最后，是因为考虑到如下的反例：

    ```
    cpu0:
    
    current = Y
    enqueue Y
    load_balance,Y放到了cpu1
    pick Z, next = Z 
    spinlock release 
    时间点1
    switch_to(Y->Z)
    
    cpu1:
    
    current = X 
    enqueue X 
    时间点1 
    pick Y，next=Y
    switch_to(X->Y)
    ```

    我们的cpu在switch的时候并没有加锁，因此在增加load_balance机制后会导致可能出现上面反例中Y进程的情形。在swtich的过程中Y进程存放寄存器信息的位置被一边读一边写，于是寄存器失效。

  - `schedule`处理流程中的enqueue(current)操作被放在了最后，该操作必须与进程被唤醒后的enqueue操作（即`wakeup_proc`函数中的enqueue）互斥。如果不满足互斥的条件，那么可能进程被`event`机制唤醒、又被另一个CPU的`timer_list`唤醒，从而导致enqueue两次而出错。 （我们的实现是，进程在`load_balance`被搬运到其他CPU后，不更改`timer_list`，只更改进程的`runqueue`，因为ucore+没有区分被时钟中断唤醒与被同步互斥机制唤醒）



## 小组成员分工



## 实现方案



## 主要代码修改描述



## 代码版本描述



## 测试场景描述及测试结果



## 实验后续开发的建议以及设想



## 实验过程日志



## 实验总结



## 其他说明