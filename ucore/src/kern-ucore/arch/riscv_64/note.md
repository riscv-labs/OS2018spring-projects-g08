# LAB 1
途径：diff risc-v与i386两个ucore的区别，diff i386与ucore+的区别。

首先把risc-v的文件拿过来，然后加ucore+。加的时候参考两个diff。
注意各个.h文件需要更改#define之类的

## 

### Debug
1. kdebug.h里保留了一部分x86的debug函数，需要删掉。
### Driver
- clock.c完全不一样，需要使用risc-v的
- console.c删掉了好多东西，需要使用risc-v的
- intr.c需要使用risc-v的
- picirq.h & c需要使用risc-v的
- **LAB1中不需要ide，但我直接把ucore+中的代码拿了过来。不确定是否会有问题。**

### Init
- init.c需要使用risc-v的，并且include<arch.h>(即riscv.h)

### mm
- memlayout.h删了x86的东西，需要用risc-v的
- mmu.h删了x86的东西，需要用risc-v的
- pmm.c删了x86的东西，需要用risc-v的

### trap
- trap.h & c需要使用risc-v的
- trapentry.S需要使用risc-v的
- **暂时忽略掉了所有kdebug里新增的中断（断点之类的）**

### Libs
- 新增riscv.h和sbi.h

### tools
- grade.sh需要使用risc-v的
- kernel.ld需要使用risc-v的

### Makefile
需要使用risc-v的。具体修改如下：
- 更改了gcc和qemu命令
- 增加了SPIKE和GDB
- 更改了CFLAGS和LDFLAGS，去掉了HOSTFLAGS里的-g
- KINCLUDE里增加了riscv.h
- 删掉了bootblock
- ucore.img的生成方式改变（最重要）
- .PHONY改变了好几处
- 删掉了all
- clean改变
- 增加了tags


- **在TARGET_CC_FLAGS_COMMON中删掉了DEFS，在原来的ucore中用于grade.sh。特此标记。**




