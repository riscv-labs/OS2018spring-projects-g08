# Group08 ucore+基于64位RISC-V的SMP移植以及ucore+的优化（ucore 3.0）大实验报告

##### 成员

计52 路橙 2015010137

计52 于志竟成 2015011275 

## 选题概述

在前人的工作中，已经将ucore成功porting到RISC-V 64上。RISC-V作为一个新兴的指令集架构正在受到越来越广泛的关注。由于实现简介、分层明确，基于RISC-V的OS也是当下的热点。我们组对ucore+的优化很感兴趣，尽管前人已将ucore+在x86上实现了一个基础的SMP（见相关工作），但由于其实现并不完善（无法通过所有测例），实现的机制比较简单（内核大锁），因此ucore+并没有对SMP有很好的支持。此外，ucore+的LKM实现很好，我们希望RISC-V的移植也可以支持LKM。因此，我们组计划基于这两个方面对ucore+进行优化，从而得到**ucore 3.0**。由于RISC-V的架构天然适合SMP，因此基于RISC-V的ucore+的SMP移植较x86而言难度更低，故我们的SMP移植将基于RISC-V 64的CPU进行。

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

我们在工作中可以参考的相关工作主要可以分为以下几个部分。

### RISC-V specifications ([http://riscv.org](http://riscv.org/)) 

- User-level ISA specifications (2.2)
- Draft Privileged ISA specifications (1.10)

### ucore及现有的ucore在RISC-V架构上的移植

- ucore for RISC-V 64 (https://gitee.com/shzhxh/ucore_os_lab)。需要切到riscv64 branch。
- ucore+ (https://github.com/oscourse-tsinghua/ucore_plus)
- bbl-ucore (https://ring00.github.io/bbl-ucore/)

### RISC-V SMP相关

- RISC-V Proxy Kernel and Boot Loader (https://github.com/riscv/riscv-pk)
- ucore SMP in x86 (http://os.cs.tsinghua.edu.cn/oscourse/OS2013/projects/U01)

### 多核优化

- *The Scalable Commutativity Rule*
  - commuter (<https://pdos.csail.mit.edu/archive/commuter/>)
  - sv6 (<https://github.com/aclements/sv6>)

### LKM相关

- 蓝昶ucore+ LKM (<http://os.cs.tsinghua.edu.cn/oscourse/OS2012/projects/U06>)
- RISC-V ELF <https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md>



## 学习笔记以及实验移植过程文档

- LKM: <https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/lkm.md>
- RISC-V pk/BBL: <https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/pk_bbl.md>
- ucore+ i386, ucore diff: [https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore%2B%20i386%20vs%20ucore.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore+%20i386%20vs%20ucore.md)
- ucore+ rv64 porting: [https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore%2B_rv64_porting.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore+_rv64_porting.md)
- RISC-V ISA: <https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/riscv_priv1.10.md>, <https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/riscv_spec2.2.md>
- SMP: <https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/smp.md>



## 小组成员分工

### 路橙

- 前期学习ucore+的架构。
- 将ucore+移植到RISC-V硬件平台上。
- ucore+ for RISCV开启SMP，并debug。

### 于志竟成

- 前期学习SMP和LKM。修ucore+上原有的LKM。
- 将LKM移植到ucore+ for RISC-V上。
- ucore+ for RISCV开启SMP，并debug。
- RISC-V print_stackframe的实现。

### 与他组合作

- 与Group03合作。我们最终的仓库为同一个仓库，最终目的是合并32bit与64bit的代码。高信龙一帮我们将我们的版本移植到了最新的ucore+上；杨国烨与我们合作SMP，他参考UNIX系统的实现，写了简化版的load_balance算法；我们做SMP的硬件部分以及SMP运行后的debug工作，最终与32位代码进行合并；此外，我们两个组的仓库的CI是杨国烨完成的。
- 与Group06进行了密切的交流，谭闻德同学帮我们组调出了**很多很多很多很多**的bug。我们的讨论过程也不时地启发谭闻德同学思考sv6移植过程中存在的问题和潜在的bug。如果没有谭闻德，就不会有我们的最终结果。十分感谢谭闻德同学！



## 实现方案

### 实验环境搭建

- 经过讨论，我们决定选择RISC-V 1.10规范。原因有二，一方面1.91的一些工具链如bbl已不再维护，另一方面前人在64位RISC-V上的搭建也是基于1.10规范。
- Linux上使用谭闻德提供的工具链，将ucore for RISC-V 64的前人移植工作跑通。macOS上基于Homebrew也完成了搭建。
- 编译ucore+ for amd64并成功运行。（需要安装libncurses5-dev）

##### RV64 for ubuntu16.04 Env Setup

- [Download ToolChain file](https://files.twd2.net/riscv/) from 谭闻德（已经编译完成）
- apt install libsdl2-2.0（libsdl2-dev会把虚拟机显卡搞崩）
- tar -xf rv64-prebuild.tgz
- export PATH=$PATH:(bin文件夹的路径) 

##### RV64 for macOS 10.13.4 Env Setup

- 使用Homebrew安装目标为RISC-V 64的交叉编译器（[https://github.com/riscv/homebrew-riscv）](https://github.com/riscv/homebrew-riscv%EF%BC%89) 
- 编译QEMU with RISC-V (RV64G, RV32G) Emulation Support（[https://github.com/riscv/riscv-qemu）](https://github.com/riscv/riscv-qemu%EF%BC%89) 

### 学习ucore+ for i386和ucore的差别

ucore+在架构上进行了一些调整，且新增了许多模块和功能，学习这些差别后才可以知道如何移植。

学习文档记录见：[https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore%2B%20i386%20vs%20ucore.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore%2B%20i386%20vs%20ucore.md)

### 学习RISC-V 64 1.10的特性

为了看懂前人的移植工作，这一点必不可少。

学习文档记录见：

- [https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/riscv_priv1.10.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/riscv_priv1.10.md)
- [https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/riscv_spec2.2.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/riscv_spec2.2.md)

### 学习BBL、学习SMP开启方法

通过研究bbl（risc-v pk），看懂了BBL大致做了什么事情，并且对RISC-V有了更深入的理解。此外，看了看BBL，也大致明白了SMP的开启方法。

学习文档记录见：

- [https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/pk_bbl.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/pk_bbl.md)
- [https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/smp.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/smp.md)

### 学习LKM相关

- 研究了LKM的基本理论，包括：
  - 更全面地学习了ELF以及相关工具的使用
  - 从用户角度了解了Linux中的LKM，认识了LKM的作用和大致的工作方式
- 研究了ucore+中的LKM实现
  - 读了一些LKM加载的代码
  - 大致扫了一圈与LKM相关的用户级工具的代码（insmod, rmmod, lsmod）
- 折腾了一下ucore+中给hello这个LKM
  - 成功编译链接生成了hello.ko
  - 分析了hello.ko的section
  - 通过进一步折腾，成功在ucore+中加载了阉割版hello.ko（删去了所有引用内核中无法解析的符号的语句）
  - 在上述过程中发现了一些似乎和实现有些矛盾问题，但是有的问题似乎又没有导致任何后果（例如`__versions`段似乎并不在hello.ko中，但是加载过程仍然顺利完成了）。过后我们会进一步研究这些问题
  - 又经过了一些折腾（调了一下LKM中struct module的格式等），成功在ucore+中加载了hello.ko阉割版2.0（添加`kprintf`取代`printk`）。现在LKM可以在加载和卸载时正常输出一些字符串
  - 研究蓝昶学长的LKM实现。

对LKM的学习笔记见：

- RISC-V ELF：(https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/elf.md)
- LKM学习及移植过程：(https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/lkm.md)

### 将RISC-V 64的ucore移植到ucore+上

有两种途径：

1. 根据前人的移植文档，将ucore+移植到RISC-V 64上。具体而言，把arch/i386拷贝一份然后根据前人的文档找到对应的代码进行更改。
2. 根据ucore+与ucore的差别，综合前人的文档，将已经在RISC-V 64上移植好的ucore迁移到ucore+上。具体而言，逐个lab进行移植，主要抓住ucore+与ucore的区别进行。

由于前人是逐个lab进行移植，而ucore+中只有最终版本的代码，若采取方案1，移植完lab1后无法测试，只能注释很多lab2~lab8的语句才可以成功编译，并且lab的测例也无法很好地利用，持续集成更是很困难。这样在最终移植结束后会有很大的不确定性，找bug的成本也大大增加。

经过分析ucore+的arch/i386和ucore的lab8_answer之间的差别，我们发现本质上差别并不很大，很适合逐个lab进行移植。这样一来也可以支持持续集成和测试，开发的难度也大大降低。因此我们组决定采用方案2进行移植。

由于采取了增量移植的方法，我们也对ucore+如何新增一个硬件有了一个很清晰的认识，并写了文档指导后人。

移植过程文档见：[https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore%2B_rv64_porting.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore%2B_rv64_porting.md)

### 在ucore+ for RISC-V上支持LKM

经过之前的前期准备工作，我们已经对LKM和RISC-V ELF有了非常详尽的了解。移植LKM for RISC-V 64的过程如下：

- LKM在RISC-V 64上能够正常地解析符号 。
- 研究RISC-V的重定位问题。 
- i386上的测试样例已经能够在RISCV上复现。移植文档见：[https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/lkm.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/lkm.md)
- SFAT文件系统可以作为一个LKM在ucore+ RISC-V 64上正常运行。

### 借鉴BBL的实现、Linux实现、xv6实现，ucore+ for SMP开启

BBL上已经有了初步的SMP支持，我们可以借鉴其中的实现在ucore上实现完善的SMP。此外，Linux已经有了RISC-V的移植，有详细的文档可以参考。

由于SMP开启较为复杂，硬件和软件都需要仔细研究。我们组与Group03组经过讨论后决定我们组实现硬件相关部分，Group03组实现软件相关部分。

软件部分借鉴xv6，但ucore+与xv6有一些区别，我们的runqueue是per cpu的，因此需要额外设计调度算法。杨国烨同学借鉴了类似UNIX的算法设计。

硬件部分借鉴Linux在RISC-V上的移植。

首先需要在不打开AP的时钟中断的情况下（即AP都是idle，只有BSP工作）使ucore+能正常运行，那这样就说明硬件相关基本实现完成。其次在load_balance实现好后，打开AP的时钟中断。

最终，我们组完成了RISC-V SMP相关的全部硬件实现，Group03组的杨国烨同学实现了load_balance算法，并参与了一些调bug工作。剩余的主要SMP调bug工作由我和于志竟成不断地推进。

###  为ucore+ for RISC-V 64添加stack trace功能

在系统开发过程中，调试本来就是一件非常麻烦的事，基本上任何信息都需要依赖于对系统所处状态的输出。调用栈的结构是系统状态的一个非常重要的方面，如果我们能够在系统出bug的地方获取到其调用栈的结构，我们就能够大致了解到在到达当前状态之前发生的一些事。

遗憾的是，现有的课程设计中支持RISC-V的操作系统均没有实现stack trace，这个问题在我们尝试对我们的系统进行调试时一直困扰着我们。因此，我们为我们的系统添加了stack trace功能。

这个功能的实现首先需要我们对RISC-V下函数调用时栈帧的切换有所了解。在RISC-V ISP文档中，我们发现有如下两个寄存器与栈帧密切相关：

* `sp` ：保存栈顶的地址
* `fp` (`s0` )：保存栈帧的起始地址

在进行函数调用时，这两个寄存器连同旧的返回地址`ra` 寄存器的值都会被放入栈中，具体的操作在https://stackoverflow.com/questions/34182330/risc-v-assembly-stack-layout-function-call中有比较清晰的描述。

根据这一套规则，我们就可以实现stack trace了。具体实现的过程参考了Linux for RISC-V的代码。

但是，实现完后我们发现有很多函数调用并不能根据栈帧trace出来。经过一番研究，我们发现问题在于编译器优化。因此，我们又参考了Linux的Makefile中的如下代码段，关闭了编译器对函数调用进行的一些优化：

```
ifdef CONFIG_FRAME_POINTER
KBUILD_CFLAGS	+= -fno-omit-frame-pointer -fno-optimize-sibling-calls
else
...
endif
```

其中`-fno-optimize-sibling-calls`这一开关关闭的编译器优化在<https://stackoverflow.com/questions/22037261/what-does-sibling-calls-mean> 中有详细的介绍。

至此，我们可以获得每一层函数调用时PC的值，虽然这对调试而言已经非常有帮助，每次手动在内核的符号中根据PC值查找所属的符号还是非常麻烦的。因此，我们尝试更进一步，让系统自动将PC地址转换成对应的符号。

在ucore/ucore+的i386版本中，地址到符号的转换是通过STABS完成的。具体地，STABS在ELF文件中提供了`.stab和`.stabstr两个sections，分别保存符号的地址以及符号名称，系统只需要在`.stab中通过二分查找找到对应的符号，再到`.stabstr`中获取该符号的名称就可以实现这个功能。

但是，经过一番尝试后，我们发现RISC-V并不支持STABS格式。只要给目标架构为RISC-V的gcc加上`-gstabs`选项，就会得到如下的报警：

```
cc1: error: target system does not support the 'stabs' debug format
```

这就比较僵硬了。我们对这个问题想到了两个解决办法：

* 使用其他RISC-V支持的调试信息格式，例如DWARF。
* 根据ELF中的`.symtab`和`.strtab`两个sections以及section表自己构造一些类似的调试信息出来。

第一种方法需要学习一种新的规范，我们大概看了一下DWARF，发现有些复杂，于是就没有采取这种方法。

第二种方法的好处是我们已经对相关的这一套东西有了比较清晰的了解，但是也有麻烦之处，在于：

* `.symtab`是会被链接器丢掉的。在链接器脚本中似乎没有办法引用输入文件中的`.symtab`。
* 现有的一些ELF工具，包括`objdump, objcopy, readelf`等，要么不支持抽取`.symtab`的内容，要么只支持奇怪的hexdump。

最后，我们通过`readelf -x`得到`.symtab`和`.strtab`的hexdump，用`sed`和`tail`对得到的文本输出进行了一些适当的编辑，然后用`xxd -r`将输出的hexdump文本反向转换成二进制文件。我们通过

```
.section .symdata
.incbin "symtab.raw"
.section .strdata
.incbin "strtab.raw"
```

这样的简单的汇编器指令将这些二进制内容加入到了ELF文件的`.symdata`和`.strdata`两个sections中，然后在链接器脚本中加入了定义了四个符号分别指向这两个sections的起止地址，以方便系统进行引用。

## 主要代码修改描述

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

移植文档见[ucore+ for RISC-V 64 单核移植文档](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/ucore%2B_rv64_porting.md)。

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

移植文档见：[https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/lkm.md](https://github.com/riscv-labs/OS2018spring-projects-g08/blob/master/docs/lkm.md)

- RISC-V的重定位。

  RISC-V作为RISC指令集，工程中很多事都需要依赖编译器和链接器的工作。对于链接器来说，由于重定位过程本质上就是将解析得到的符号值填回代码中，而RISC-V需要依赖多条指令来实现对一个过长的立即数的加载，链接器就需要支持多种不同的情形，例如load一个符号、call一个符号等等。我尝试寻找一份对RISCV重定位类型的较为清晰完整的文档，但是我能够找到的与此相关的资料都不能满足这个要求，很多地方都需要我自己结合一些简单的实验来理解。最开始只想到可以参考RISCV GNU toolchain的实现，而没有想到可以参考linux的RISCV移植。Toolchain中的实现了所有标准的重定位类型，而且考虑了linker relaxation，因此比较繁琐，而linux中仅实现了少数几个需要用到的重定位类型，且没有考虑linux relaxation，因此也非常好读。

- 发现并修复原来LKM实现中的一个bug。在A依赖于B时，若先加载B，再加载A，则在解析A用到的B提供的符号时会出现访问零地址的错误。这是由于这时需要对LKM维护的依赖列表（`modules_which_use_me`）进行更新，而原来的实现在这一步中没有进行必须的初始化操作。 

- SFAT文件系统已经可以作为一个LKM在ucore+ RISC-V 64上正常运行。

  为了给LKM提供一个更强的样例（之前一直是Hello World级别的样例），证明目前的LKM实现已经达到可以投入实用的程度，我们将SFAT文件系统移植成了我们的ucore+ RISC-V 64系统的一个LKM（SFAT的代码取自蓝昶学长的代码仓库）。整个移植过程是比较清晰的，文件系统的内部实现本身几乎不需要任何改动，只需要调整接口，使内核在初始化模块时能够调用指定的初始化函数，在卸载模块时能够调用指定的卸载函数即可。由于SFAT文件系统需要使用内核中的符号，在完成上面的工作后，我还需要将其所有用到的符号通过`EXPORT_SYMBOL`宏加入到ksymtab中，使加载LKM时这些必要的符号能够得到解析。在进行移植的过程中我发现编译器在生成的内核模块重定位表中放入了`R_RISCV_BRANCH`和`R_RISCV_JAL`两种在之前的样例中没有出现过的重定位，因此我又添加了这两种重定位的实现。在调试的过程中我注意到：由于LKM和内核共用了一些头文件，而头文件中可能包含和宏定义等相关的内容，在编译LKM时有必要使用和编译内核时相同的一套宏定义，否则LKM和内核之间的差异可能会导致一些奇怪的现象（例如同一个结构体在内核和LKM中的大小不一致等等） 



### ucore+ for RISC-V 64bit上SMP的开启

我们已经将RISC-V 64bit开启SMP的硬件相关工作全部完成，且支持LKM。软件相关工作只剩下一个较明显的同步互斥相关的bug。

单核情况下，测例通过39/39.

多核情况下，测例通过38/39，`primer3`有时候无法通过。（注：`matrix`测例在核数大于等于4时无法通过，因为该测例会检查`pid = 4`的进程的输出。对于多核而言，核数大于等于4时，`pid=4`的进程为idle进程，不可能有输出）

#### 硬件相关的开启

- 内核栈需要每个CPU一个。因此需要修改`entry.S`

- 原子操作的实现使用了`gcc`的内置函数`gcc atomic builtin`，更安全可靠。但RISC-V需要保证原子操作指令访问的数据的地址8字节对齐。

  - 需要在`ARCH_RISCV64`的情形下，`typdef bool unsigned long long`。
  - 文件系统`sfs.h`中`struct sfs_inode`中`flags`是32 bit的，没有对齐，将`ino`和`flags`改成64bit的整数。 

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



## 代码版本描述

目前最新的代码在：[https://github.com/riscv-labs/OS2018spring-projects-g08](https://github.com/riscv-labs/OS2018spring-projects-g08)

而配了CI、与最新ucore+结合的版本在：[https://github.com/cxjyxxme/ucore_plus](https://github.com/cxjyxxme/ucore_plus)



## 测试场景描述及测试结果

可以直接使用CI自动集成。而手动跑测例的方法如下：

##### Linux

1. Configure:

```
  make menuconfig ARCH=riscv_64
```

or

```
  make ARCH=riscv_64 defconfig
```

We temporarily use NUMA settings in menuconfig to set the SMP config. (We can't enable NUMA in risc-v.) If you want to enable SMP, make sure the **CPU number in NUMA settings is N** (default is 1), which means the number of processors.

2. Create file system

```
  make sfsimg
  make sfsimg2
```

Here we use sfsimg2 for SFAT file system, which is an example to prove the correctness of our LKM in risc-v.

3. (Optional) create swap file If you define "Support for paging of anonymous memory(swap)" in menuconfig, then run:

```
  make swapimg
```

4. Create kernel

```
  make
```

5. Run ucore

```
  ./uCore_run -c -d obj
```

6. (Optional) Test ucore

```
  ./uCore_test -d obj
```

##### macOS

原来的测试脚本不能在macOS上正常工作，原因在于：

- macOS上不能通过`/proc/cpuinfo`获取处理器信息 
- macOS上的xargs是遵从BSD的参数命名，但是脚本中使用了与之不兼容的参数名（GNU的xargs两种都支持）
- macOS上默认的sed是BSD sed，但脚本中使用的是仅适用于GNU sed的语法 

因此，macOS的测试环境配置为：

```
 brew install gnu-sed
 SED=gsed ./uCore_test -d obj
```

##### 

## 实验后续开发的建议以及设想

- 修复SMP相关bug

- RV64 print_stackframe加上符号

- 修复LKM的一个潜在bug：

  ucore+ VFS在与LKM结合时：

  1. 在删除文件系统注册（unregister_filesystem）中，似乎并没有判断将要移除的文件系统是否正在被使用。
  2. 在移除LKM时，尽管内核检查了内核模块间的符号引用关系，但没有检查一些其他形式的依赖关系。例如，内核没有检查已经注册的文件系统是否属于将要移除的模块，文件系统的卸载完全依赖于内核模块cleanup自觉进行处理。这导致已经被卸载的模块的数据可能仍被内核引用（甚至是直接执行代码），对系统的安全性构成威胁。

- 将RV64和RV32的代码合并。

- 跑通硬件上的RV64。

- 尝试将sv6的一些特性移植到ucore+，改进ucore+的并行性能。 



## 实验总结

本次实验我们耗费了很多的时间，对RISC-V、LKM、SMP三者都有了一个十分清晰的认识。实现的过程本质上就是debug的过程，通过不断地debug来让自己的理解变得更深刻。

感谢陈渝老师、向勇老师对于我们课程设计的耐心指导；

感谢杨国烨、高信龙一组和我们的合作和讨论；

特别特别感谢谭闻德同学的大力帮助。