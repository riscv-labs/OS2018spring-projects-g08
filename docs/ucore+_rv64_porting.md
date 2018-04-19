# LAB 1

途径：diff risc-v与i386两个ucore的区别，diff i386与ucore+的区别。

1. 首先将rv64的各个文件仿照ucore+ i386的排布放好，放的时候需要参考rv64与ucore_os_lab的区别，来确定哪些需要保留。
2. 然后通过i386与ucore+的区别来更换rv64中需要和ucore+适配的部分，同时需要参考rv64与ucore_os_lab的区别，来确定哪些需要保留与替换。

注意各个.h文件需要更改#define之类的（有些头文件的#ifndef里的内容和头文件名字不同，ucore+和ucore中同样的文件也不同）

## Makefile

1. 执行`make ARCH=riscv_64 defconfig`，会读取`ucore/src/kern-ucore/arch/riscv_64`中的`configs`文件夹下对应的config。由于lab1中没有内存管理、进程调度、文件系统，因此**将config文件全部注释。**

2. 执行`make`时需要交叉编译器的指定，指令为`make CROSS_COMPILE=riscv64-unknown-elf- `。

3. `make`后会生成kernel.img，主要过程是在`ucore/src/kern-ucore/Makefile.build`和同文件夹下的`Makefile`中定义的。由于ucore中把调度、文件系统、同步互斥等大部分放在了arch无关的位置，lab1中不可以编译这些文件。因此，修改`ucore/src/kern-ucore/Makefile`如下：

   ```makefile
   #dirs-y := schedule syscall fs process mm libs sync kmodule sysconf numa

   dirs-y := mm libs
   ```

4. 接着会进入`ucore/src/kern-ucore/`中对应的各个子文件夹（即上面`dirs-y`指定的文件夹）进行编译。需要修改各个文件夹下的文件：

   1. 由于Lab1中的`mm`需要用Lab1特有的，因此`ucore/src/kern-ucore/mm/Makefile`修改为：

      ```makefile
      # obj-y := pmm.o shmem.o swap.o vmm.o refcache.o
      # obj-$(UCONFIG_HEAP_SLAB) += slab.o
      # obj-$(UCONFIG_HEAP_SLOB) += slob.o
      ```

   2. `ucore/src/kern-ucore/libs`中主要有两点修改：

      1. `clock.h`中增加声明`void clock_set_next_event(void);`，这是risc-v移植时独有的。

      2. `Makefile`修改为：

         ```makefile
         #obj-y := hash.o printfmt.o rand.o rb_tree.o readline.o string.o bitset.o
         obj-y := printfmt.o readline.o
         ```

         这是因为，通过参考ucore_rv64的libs（**一共有两个文件夹**），对比后得出需要保留这些。`string.o`在ucore+中需要后续lab的支持，因此lab1中需要自己添加libs（在后面提到）。

5. 接着会执行`ucore/src/kern-ucore/Makefile.subdir`，进入`ucore/src/kern-ucore/arch/riscv_64`中进行编译：

   1. `include.mk`修改：

      ```makefile
      ARCH_INLUCDES := . debug driver include libs mm numa process sync trap syscall kmodule driver/acpica/source/include
      ARCH_CFLAGS := -mcmodel=medany -std=gnu99 -Wno-unused -Werror \
      				-fno-builtin -Wall -O2 -nostdinc \
      				-fno-stack-protector -ffunction-sections -fdata-sections
      ARCH_LDFLAGS := -m elf64lriscv -nostdlib
      ```

      `ARCH_INCLUDES`沿用了`i386`中的代码（**TODO：还没有看懂**）

      `ARCH_CFLAGS`和`ARCH_LDFLAGS`使用了`ucore_rv64`的`makefile`下的`CFLAGS`和`LDFLAGS`。

      **需要注意的是，ldflags去掉了--gc-sections（这个指令可以优化编译结果的大小），因为加上的话会报错。没有搞清楚原因（TODO）。**

   2. `Makefile`修改：

      ```makefile
      # dirs-y := debug driver init libs mm numa process syscall trap kmodule
      dirs-y := debug driver init libs mm trap
      ```

   3. `Makefile.image`沿用`ucore_rv64`中`UCOREIMG`的生成代码：

      ```makefile
      $(KERNEL_IMG): $(KERNEL_ELF)
      	@echo Making $@
      	cd $(TOPDIR)/../riscv-pk && rm -rf build && mkdir build && cd build && ../configure --prefix=$(RISCV) --host=riscv64-unknown-elf --with-payload=$(KERNEL_ELF) --disable-fp-emulation --enable-logo && make && cp bbl $(KERNEL_IMG)
      ```

      其中`riscv-pk`文件夹放在了`ucore`文件夹的平行路径下。第一次运行时需要进入`riscv-pk`中执行`chmod +x configure`。

   4. `ucore.ld.in`需要改成`ucore_rv64`的`tools/kernel.ld`。

   5. `ucore.mk`直接用了`i386`的文件，**TODO：还没看懂**。

   6. `Kconfig`直接用了`i386`的文件，**TODO：还没看懂**。

   7. 在各个子文件夹下增加`Makefile`文件，指定需要编译的c文件。

6. 此外，为了成功执行`make qemu`，需要在`ucore/Makefile`下添加：

   ```makefile
   qemu: kernel
   	$(Q)qemu-system-riscv64  -machine virt -kernel obj/kernel.img -nographic

   spike: kernel
   	$(Q)qemu-system-riscv64 obj/kernel.img

   PHONY += qemu spike
   ```

   ​

最终编译的过程为：

```
cd ucore
make ARCH=riscv_64 defconfig
make CROSS_COMPILE=riscv64-unknown-elf-
make qemu CROSS_COMPILE=riscv64-unknown-elf-
```

即可看到100ticks的不断弹出（同Lab1）



## riscv_64下各个文件夹（ucore_rv64 vs ucore_os_lab）

**注意：在把ucore_rv64移植到ucore+上后，还需要参考ucore+ i386和ucore_os_lab的区别来更改一些头文件或者语句描述等（见另一个文档）。**

### Debug

#### kdebug.h & kdebug.c

1. 需要用`rv64`的版本。
2. **TODO：这里的移植没有完成ucore+新增的kdebug，如断点等。**

#### monitor.h & monitor.c

1. 需要用`rv64`的版本。
2. **TODO：这里的移植没有完成ucore+新增的kdebug，如断点等。**

#### panic.c & stab.h

用`rv64`的版本。

### Driver

- `clock.c`完全不一样，需要使用`rv64`的

- `console.c`删掉了好多东西，需要使用`rv64`的

- `intr.c`需要使用`rv64`的

- `picirq.h & c`需要使用`rv64`的

- **TODO：LAB1中不需要ide，但我直接把ucore+中的代码拿了过来。不确定是否会有问题。**



### Init

- init.c需要使用risc-v的，并且include<arch.h>(即riscv.h)
- `entry.S`使用`rv64`的。
- `init.c`使用`rv64`的，对于ucore+新增加的特性没有用。即：**TODO：mp_init（NUMA）、debug_init（kdebug新增加的内容）、mod_init（module）**。



### mm

- `memlayout.h`删了x86的东西，需要用risc-v的

- `mmu.h`删了x86的东西，需要用risc-v的

- `pmm.c`删了x86的东西，需要用risc-v的



### trap

- `trap.h & c`需要使用risc-v的

- `trapentry.S`需要使用risc-v的

- **TODO：暂时忽略掉了所有kdebug里新增的中断（断点之类的）**



### Libs

- `arch.h`使用`rv64`的`riscv.h`
- 把ucore+ i386下的libs拷贝了过来
- `kio.c`（沿用自ucore+ i386）需要注释`EXPORT_SYMBOL(kprintf);`，否则会编译报错。（**未清楚原因（TODO）。以后需要调用kprintf时需要#include<kio.h>**。此外，删掉了同步互斥的一些机制，后续需要补充**（TODO）**
- `sbi.h`沿用自`rv64`，需要把`uint_t`改为`uint64_t`。
- `string.h & string.c`使用`rv64`。（ucore+的无法使用，因为没有lab2的支持）。
- `Makefile`只需要：`obj-y := kio.o string.o`



### ucore_rv64 vs ucore_os_lab的Makefile区别

`rv64`具体修改如下：

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









