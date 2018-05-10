# RISC-V多核

* `mhartid`：当前所处的硬件线程（hart）的id（一定有一个hart的id为0）
*  `amo...`：原子操作，可以用于实现资源的互斥访问

## Inter-Processor Interrupts (IPI)

* 每个mode有一个自己的context

# pk/bbl

* 0号线程先进行初始化，其余线程用`wfi`等待0号线程的软件中断后再进行初始化。只有id在0到`MAX_HARTS-1`之间且未被mask掉的线程最终能够得到初始化。0号线程在初始化时，做的事情不只是初始化线程本身，还需要初始化系统整体的一些资源和环境。
* bootloader过程分两个阶段，先从M`mret`到S，然后再S里面加载ELF，做进一步设置，再`sret`到入口位置执行
* 0号之外的其他线程初始化之后就进入不断`wfi`的状态（等之后的IPI让它们干活？）

# xv6

# Linux RISC-V

## 启动

参考：https://www.sifive.com/blog/2017/10/09/all-aboard-part-6-booting-a-risc-v-linux-kernel/

bbl启动每个hart后，会让每个hart跳转到操作系统的入口处，并设置`a0`、`a1`分别为hartid和设备树地址。

为了保证仅有一个hart进行一些公共的系统初始化工作（例如初始化内存管理等），Linux RISC-V在这里使用了一个被称作hart lottery的机制，即使用一个原子read-modify-write操作来改写一个内存单元。第一个执行这个指令的hart会得到`a3=0`，它被选做进行这些初始化的hart：

    	/* Pick one hart to run the main boot sequence */
        la a3, hart_lottery
        li a2, 1
        amoadd.w a3, a2, (a3)
        bnez a3, .Lsecondary_start

必要的初始化完成后，其他的hart就可以继续执行了。其他hart什么时候知道自己可以继续执行了呢？这件事其实需要某种hart间通信的机制，在这里Linux RISC—V用了一种简单粗暴的轮询方式来达到这个目的：

        .Lwait_for_cpu_up:
            /* FIXME: We should WFI to save some energy here. */
            REG_L sp, (a1)
            REG_L tp, (a2)
            beqz sp, .Lwait_for_cpu_up
            beqz tp, .Lwait_for_cpu_up
            fence

可以发现事实上hart是在等待两个特定的地址被改写，而这两个地址的值将作为`sp`和`tp`寄存器。`sp`就是存放栈顶位置的寄存器，所以hart其实就是在等待自己的内核栈被建立好。`tp`寄存器存放的是`task_struct`的指针，这个`task_struct`维护了任务（线程）的信息，包括目前在哪个CPU上、优先级、各自的内存管理等（应该就是个进程控制块），每个的hart的hartid就是通过`tp`寄存器保存的`task_struct`维护的：

        #define raw_smp_processor_id() (*((int*)((char*)get_current() + TASK_TI_CPU)))

        static __always_inline struct task_struct *get_current(void)
        {
            register struct task_struct *tp __asm__("tp");
            return tp;
        }

`smpboot.c`中：

        __cpu_up_stack_pointer[cpu] = task_stack_page(tidle) + THREAD_SIZE;
        __cpu_up_task_pointer[cpu] = tidle;

        while (!cpu_online(cpu))
            cpu_relax();
    
这里的`__cpu_up_stack_pointer`和`__cpu_up_task_pointer`是保存每个hart启动时使用的`sp`和`tp`的数组，hart轮询等待改写的就是这两个数组的内容。

这里的`cpu_online`检查了`cpu_online_mask`中对应位。除了`cpu_online_mask`外还有`cpu_present_mask`、`cpu_active_mask`和`cpu_possible_mask`负责维护每个hart的状态，这些`cpumask`的定义如下：

        typedef struct cpumask { DECLARE_BITMAP(bits, NR_CPUS); } cpumask_t;

四个`cpumask`的说明：

 * cpu_possible_mask- has bit 'cpu' set iff cpu is populatable
 * cpu_present_mask - has bit 'cpu' set iff cpu is populated
 * cpu_online_mask  - has bit 'cpu' set iff cpu available to scheduler
 * cpu_active_mask  - has bit 'cpu' set iff cpu available to migration

TODO:研究`smpboot.c`中的`__cpu_up`是在哪里调用的。


# xv6

xv6的进程表似乎是所有hart共享的。在idle中，每个hart都会等待spinlock，获取到spinlock后选中进程表中的一个进程进行执行。
