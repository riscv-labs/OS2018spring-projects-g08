# ucore+基于64位RISC-V的SMP移植以及ucore+的优化（ucore 3.0）

## 选题概述

在前人的工作中，已经将ucore成功porting到RISC-V 64上。RISC-V作为一个新兴的指令集架构正在受到越来越广泛的关注。由于实现简介、分层明确，基于RISC-V的OS也是当下的热点。我们组对ucore+的优化很感兴趣，尽管前人已将ucore+在x86上实现了一个基础的SMP（见相关工作），但由于其实现并不完善（无法通过所有测例），实现的机制比较简单（内核大锁），因此ucore+并没有对SMP有很好的支持。此外，ucore+的底层硬件抽象层的实现也不够通用。ucore+的这两点存在很大的优化可能性。因此，我们组计划基于这两个方面对ucore+进行优化，从而得到**ucore 3.0**.由于RISC-V的架构天然适合SMP，因此基于RISC-V的ucore+的SMP移植较x86而言难度更低，故我们的SMP移植将基于RISC-V 64的CPU进行。

具体而言，我们组的任务分为如下四部分：
1. **移植**。将ucore在RISC-V上的移植迁移至ucore+上。
2. **开启SMP**。将移植好的ucore+开启SMP机制。
3. **优化SMP**。将初步开启SMP机制的ucore+中的热加载模块进行优化，从而使动态内核模块适用于SMP架构。
4. **优化硬件抽象层**。通过学习Linux硬件抽象层的实现，更改ucore+的硬件抽象层以获得更高的适配性。

## 相关工作

我们在工作中可以参考的相关工作主要可以分为以下几个部分。

### RISC-V ISA文档

* User-level ISA specifications (2.2)
* Draft Privileged ISA specifications (1.10)

### ucore及现有的ucore (plus)在RISC-V架构上的移植

* ucore RISC-V 64 (https://gitee.com/shzhxh/ucore_os_lab)
* ucore+ (https://github.com/chyyuu/ucore_os_plus)
* bbl-ucore (https://ring00.github.io/bbl-ucore/)

### RISC-V SMP相关

* RISC-V Proxy Kernel and Boot Loader (https://github.com/riscv/riscv-pk)
* ucore SMP in x86 (http://os.cs.tsinghua.edu.cn/oscourse/OS2013/projects/U01)

### 多核优化

* sv6 (https://github.com/aclements/sv6)

### HAL相关

* The Linux Kernel (http://kernel.org)


## 实验方案

我们计划按照如下的步骤进行实验：
### 实验环境搭建——已完成
- 经过讨论，我们决定选择RISC-V 1.10规范。原因有二，一方面1.91的一些工具链如bbl已不再维护，另一方面前人在64位RISC-V上的搭建也是基于1.10规范。
- Linux上使用谭闻德提供的工具链，将ucore for RISC-V 64的前人移植工作跑通。macOS上基于Homebrew也完成了搭建。
- 编译ucore+并成功运行。（需要安装libncurses5-dev）

### 学习ucore+和ucore的差别——预计本周末前完成
ucore+在架构上进行了一些调整，且新增了许多模块和功能，学习这些差别后才可以知道如何移植。

### 学习RISC-V 64 1.10的特性——预计本周末前完成
为了看懂前人的移植工作，这一点必不可少。

### 将RISC-V 64的ucore移植到ucore+上——预计第10周周末前完成
有两种途径：
1. 根据前人的移植文档，将ucore+移植到RISC-V 64上。具体而言，把arch/i386拷贝一份然后根据前人的文档找到对应的代码进行更改。
2. 根据ucore+与ucore的差别，综合前人的文档，将已经在RISC-V 64上移植好的ucore迁移到ucore+上。具体而言，逐个lab进行移植，主要抓住ucore+与ucore的区别进行。

由于前人是逐个lab进行移植，而ucore+中只有最终版本的代码，若采取方案1，移植完lab1后无法测试，只能注释很多lab2~lab8的语句才可以成功编译，并且lab的测例也无法很好地利用，持续集成更是很困难。这样在最终移植结束后会有很大的不确定性，找bug的成本也大大增加。

经过分析ucore+的arch/i386和ucore的lab8_answer之间的差别，我们发现本质上差别并不很大，很适合逐个lab进行移植。这样一来也可以支持持续集成和测试，开发的难度也大大降低。因此我们组决定采用方案2进行移植。

### 借鉴BBL的实现、xv6的SMP实现以及前人在amd64上开启SMP的实现，将ucore在RISC-V 64上开启SMP。
BBL上已经有了初步的SMP支持，我们可以借鉴其中的实现在ucore上实现完善的SMP。此外，前人已经在ucore+的amd64上尝试过SMP的开启，尽管不完善，但也有一定的借鉴作用。

由于开启SMP和ucore+的移植是可以并行的工作，因此我们决定首先在ucore上开启SMP，当ucore+的移植结束后再将该工作迁移到ucore+上。

### 在ucore+在RISC-V 64上开启SMP——预计第十周周末前完成
由于ucore+有动态内核加载模块，故开启SMP时可能会存在一定的问题。在这一步的实现时我们可能会关闭动态内核加载的功能。

### 将ucore+上的动态内核加载功能支持SMP——预计第十三周汇报前完成
动态内核加载模块是ucore+上的一个很有意义的工作，但当初的实现假定了单核CPU，在开启SMP后可能会存在问题。因此，尝试更改动态内核加载模块以适配SMP，是完成前述工作后的一项很有挑战的工作。

### 优化ucore+的底层硬件抽象层实现——预计在最终报告前得到一定进展
考虑到我们在完成前述工作后对ucore+的架构已有一定程度的深入认识，因此我们还可以考虑对ucore+的底层硬件抽象层进行优化，仿照Linux的实现，以完成更通用、更高效的实现。

### 与bug组同学交流，尝试找出我们实现的bug——预计在最终报告前得到一定进展
这是一项很有意义的合作。

## 小组分工
### 路橙
完成ucore的RISC-V 64到ucore+的迁移工作。
完成动态内核加载模块对SMP的支持。
优化ucore+的底层硬件抽象层实现。

### 于志竟成
完成ucore的RISC-V 64的SMP开启。
完成ucore+的RISC-V 64的SMP开启。（关闭动态内核加载功能）
优化ucore+的底层硬件抽象层实现。

## 已完成工作

目前主要完成的工作包括开发环境的搭建以及相关知识的初步学习。

### 开发环境搭建

我们已经完成了开发环境的搭建工作，包括：

* 安装RISC-V 64工具链
* 编译并成功运行ucore+ x86\_64以及ucore RISC-V 64

### ucore+的整体架构学习
见项目仓库docs下ucore+ i386 vs ucore。

### ucore+ x86 smp前人工作的运行
已可以成功运行。跑测例会崩，因为前人没做完。

### RISC-V指令集的初步学习

通过结合对ucore for RISC-V 64及bbl-ucore代码的学习和对RISC-V ISA文档的认识，我们对RISC-V的整体架构以及一些重要的机制（包括特权级、中断、页表等）和相关指令有了大致的了解。

### SMP的初步学习

通过阅读RISC-V pk的代码，我们了解了在RISC-V架构中与多核相关的机制，对如何在RISC-V架构中支持SMP有了基本的认识。



