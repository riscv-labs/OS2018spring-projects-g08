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




### Init

- init.c需要使用risc-v的，并且include<arch.h>(即riscv.h)
- `entry.S`使用`rv64`的。
- `init.c`使用`rv64`的，对于ucore+新增加的特性没有用。即：**TODO：debug_init（kdebug新增加的内容）、mod_init（module）**。



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
- `kio.c`（沿用自ucore+ i386），删掉了同步互斥的一些机制，后续需要补充**（TODO）**
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




# Lab 2

## Lab2与Lab1的区别（rv64）

注：**tools/vector.c没用**。

#### kmonitor.c

1. 新增`bool is_kernel_panic(void);`的声明

#### kmonitor.h

1. 新增了几个函数声明

#### clock.c

1. 修改了一句bug的话：`__asm__ __volatile__("rdtime %0" : "=r"(n));`

#### defs.h

1. 删掉了一些定义。

#### 新增加文件

1. `default_pmm.h & c`，使用的是未经谭闻德优化的版本。在ucore+是buddy system，因此做法为：

   - 不使用`default_pmm`，使用`arch/i386/mm/buddy_system`的两个文件。`Makefile`中增加对应的编译项。
   - 将`pmm.c`中对应语句更换为`buddy_pmm_manager`
   - `memlayout.h`中`struct Page`增加`buddy_system`需要的数据结构。

2. `memlayout.h`加了很多

3. `mmu.h`加了很多

4. `pmm.h & c`加了很多

5. `sync.h`。

   **TODO：ucore+中增加了sync_init，amd64中用到，消息盒子之类的，这里init.c里暂时没有用到**。

6. `atomic.h`

7. `list.h`，与ucore_os_lab一样

#### trap.c

1. 修改了一句bug：

   ```c++
   case IRQ_U_TIMER:
   	cprintf("User Timer interrupt\n");
   ```

#### kernel.ld

增加了`BASE`



## RV64移植

```
For non-leaf PTEs, the D, A, and U bits are reserved for future use and must be cleared by software for forward compatibility
```

在手册上有这样一句话，因此需要做如下修改：


1. 需要修改`pte`操作函数、`get_pte`时对保留位的控制、以及去掉一条错误的`assert`（与`PTE_U`相关）。
2. `print_pgdir`和`get_pgtable_items`是有bug的！因为没有考虑高级页表直接作为leaf table的情况。(**TODO**)
3. ucore+使用的多级页表是模板，因此需要在`mmu.h`中定义一个假的4级页表来模拟risc-v 64bit的3级页表（第4级和第3级相同）。这样的做法可以让之后risc-v 64bit与32bit的代码合并具有很大的便利。
4. 卡在`enable_paging`出不来，最后发现是因为`mp_init`没有打开。因为ucore+的`numa`机制是通用的模板，需要适配这个。
5. risc-v 64bit的多级页表，对于none-leaf page table而言，需要R、W、X位都为0（期中考试也有涉及），且参考risc-v 1.10手册可知D、A、U位也需要置零。因此需要修改ucore+下通用`pmm.c`的`get_pte`，加上risc-v硬件的判断。
6. risc-v的leaf page table entry中，如果W为1，那么R也必须为1，否则CPU会报错。因此设置页表项的W、R的函数必须进行相应修改。（补充：后续设置`vm_flags`时也需要相应修改）
7. risc-v中页表项和物理地址的长度不同，因此设置页表项对应的物理地址时不能像x86那样简单地进行进行或操作，需要移位。





# Lab 3

## Lab3与Lab2的区别（rv64）

#### Makefile

1. CFLAGS改变：增加了`-s `，去掉了`-Wno-unused -Werror`，换成了`-Wextra`。**我的做法中没有加`-s`。**

#### kdebug.c

1. 改了一些语句。

#### console.c

1. 增加了输出时关闭中断的机制。

#### init.c

1. 增加了：

   ```c++
   #include <vmm.h>
   #include <ide.h>
   #include <swap.h>

   ....
       
   ide_init();                 // init ide devices
   swap_init();                // init swap
   ```

#### memlayout.h

1. 增加了`swap`相关的entry和`struct Page`中关于页置换算法的数据。
   - 使用ucore+的机制。

#### pmm.c

1. `alloc_pages`增加了页换出机制
   - 由于lab2移植时使用的ucore+机制，因此这里不需要动。
2. 修了`page_insert`的一个bug
   - 由于lab2移植时使用的ucore+机制，因此这里不需要动。
3. 修了`check_boot_pgdir`的bug
   - 由于lab2移植时已经修复，故不需要动
4. 修改了`tlb_invalidate`
   - 感觉是rv64在这个lab的bug，不需要动。
5. 增加了`pgdir_alloc_page`、`kmolloc`、`kfree`
   - 使用ucore+的机制。注意pgdir_alloc_page与swap有关。
   - `kmalloc`和`kfree`使用slab。

#### trap.c

1. `idt_init`增加了`SSTATUS_SIE`和`SSTATUS_SUM`的设置
2. 增加了`page_fault`的处理
3. `exception_handler`、`trap`有修改。

#### trapentry.S

1. 修了一些bug

#### riscv.h

加了一堆东西。直接使用lab8的头文件。

#### 新增加文件

1. `ide.h &c`。

   - 直接使用ucore+中带有文件系统的版本。

2. `fs.h`，`swapfs.h & c`，由于没有文件系统，因此需要用简化版本，即`rv64`的版本。

3. `swap.h & c`，使用ucore+版本。

   - 由于ide是fake的，因此这里需要把`swap_init`中的`max_swap_offset`的判断从512改成7.**TODO:lab8中修复这个bug。**

4. `swap_fifo.h & c`，清掉，使用ucore+版本。

5. `vmm.h & c`：

   vmm.h中`mm_struct`比ucore+多了一个`sm_priv`，还多了如下声明：

   ```c++
   extern volatile unsigned int pgfault_num;
   extern struct mm_struct *check_mm_struct;
   ```

   `vmm.c`和ucore+的差别很大，需要仔细研究。

6. `rand.c`，直接用ucore+的libs即可。

7. `stdlib.h`，直接用ucore+的libs即可。




## 移植过程遇到的问题

- `TEST_PAGE`需要修改，起始地址不能太低。
- 移植`swap`时需要`ide.c`，暂时使用了ucore下labanswer中lab3的`ide.c`，本质上使用了一个全局变量数组来存数据。
- 移植`swap`的`check`函数时，需要注意risc-v的虚拟地址只有低48位有效，更高位是保留位，因此在建立第一个三级页表项下所有的页映射时，循环的条件修改为`while (!((addr0 >> 39) & 0x1FFFFFF))`，即只取三级页表对应的虚拟地址的位。
- 移植`swap`的`check`函数时，需要把`PTSIZE`等按照`x86 amd64`的类型改成`PUSIZE`、`PMSIZE`等，即使用通用的多级页表机制，方便之后增加32 bit代码。





## mm(ucore+/i386 vs ucore_os_lab)

### memlayout.h

- `ifdef`名字改了

- 增加了`#define PBASE   KERNBASE`

- 删掉了如下声明:

  ```c++
  typedef uintptr_t pte_t;
  typedef uintptr_t pde_t;
  typedef pte_t swap_entry_t; //the pte can also be a swap entry
  ```

- struct Page变化：

  ```c++
  int ref;   ->   atomic_t ref;

  针对swap算法的更改：
      list_entry_t pra_page_link;     // used for pra (page replace algorithm)
      uintptr_t pra_vaddr;            // used for pra (page replace algorithm)
  更换为：
  	swap_entry_t index;	// stores a swapped-out page identifier
  	list_entry_t swap_link;	// swap hash link
  ```

- 页表项增加如下标志位，并且增加了对应的set和clear函数：

  ```c++
  #define PG_slab                     2	// page frame is included in a slab
  #define PG_dirty                    3	// the page has been modified
  #define PG_swap                     4	// the page is in the active or inactive page list (and swap hash table)
  #define PG_active                   5	// the page is in the active page list
  #define PG_IO                       6	//dma page, never free in unmap_page
  ```

  即支持slab算法的kmalloc、更一般的页置换算法等。

### kmalloc.h & kmalloc.c

删除掉，放在了`slab`和`slob`中。

### mmu.h

- 增加了更多级页表的define，而在`kern-ucore/mm/pmm.c`中`get_pte`里要用到这些东西，所以得定义。

- 增加了如下define：

  ```c++
  #define SECTSIZE        512	// bytes of a sector
  #define PAGE_NSECT      (PGSIZE / SECTSIZE)	// sectors per page

  //上面两个在fs.h中已经定义，故不需要。

  #define PTE_SWAP        (PTE_A | PTE_D)
  ```

- 增加了关于pte的一系列inline的标志位操作函数。

#### rv64移植：

- PTE_PWT没用，只有arm用到
- PTE_PCD没用
- PTE_PS没用
- PTE_MBZ没用
- PTE_AVAIL没用
- inline函数有些没有使用。i386和x86中的这些函数的实现是错误的，但没有被使用上，所以无所谓。**TODO:fix this bug**。

### pmm.c

#### kern-ucore/arch/i386/mm/pmm.c

- 增加了如下include：

  ```c++
  #include <slab.h>
  #include <proc.h>
  #include <kio.h>
  #include <mp.h>
  ```

- 增加了

  ```c++
  static DEFINE_PERCPU_NOINIT(size_t, used_pages);
  DEFINE_PERCPU_NOINIT(list_entry_t, page_struct_free_list);
  ```

  的函数声明

- `alloc_pages`的变化：

  - 把while(1)循环换成了`#ifdef UCORESWAP`的机制。`check_mm_struct`的`swap_out`换成了`try_free_pages(n)`，后者在`kern-ucore/mm/swap.c`里。
  - 增加了`get_cpu_var(used_pages) += n;`，用于NUMA机制

- `free_pages`增加了`get_cpu_var(used_pages) -= n;`

- 多了`nr_used_pages`，用户NUMA机制

- 多了`pmm_init_ap`，用户NUMA机制

- `boot_map_segment`中修改了语句：

  ```c++
  *ptep = pa | PTE_P | perm;

  更换为：

  ptep_map(ptep, pa);  //这里会自动把present位置1
  ptep_set_perm(ptep, perm);
  ```

- `pmm_init`里将中途的映射改成了`8M`，因为kernel size比较大。还把`kmalloc_init`换成了`slab_init`。

  - rv64 porting：**TODO:print_pgdir**

- `get_pte、get_page、page_remove_pte、unmap_range、exit_range、copy_range、page_remove、page_insert、pgdir_alloc_page`放在了`kern-ucore/mm/pmm.c`下。

- 增加了`set_pgdir`、`load_pgdir`、`map_pgdir`这三个操作函数。



#### kern-ucore/mm/pmm.c

- `get_pte`：把`mmu.h`中多出来的补上就能用

- `get_page`：把`mmu.h`中多出来的补上就能用

- `page_remove_pte`：

  - 需要对每个page维护`PG_DIRTY`、`PG_IO`和`PG_SWAP`才可以使用。
  - 需要引入arch下的`NUMA`，因为调用了`mp_tlb_invalidate`。
  - 若`UCONFIG_SWAP`，还需要`kern-ucore/mm/swap.c`的`swap_remove_entry`

- `page_insert`：把上面的实现了就能用

- `page_remove`：一模一样

  **TODO：以下在lab2中没有。**

- `pgdir_alloc_page`：去掉了新页有关swap初始化的过程，因为swap机制不同。

- `unmap_range`：把上面的实现了就能用

- `exit_range`：把上面的实现了就能用

- `copy_range`：实现了COW机制，如果原进程的页在内存中，新进程会有新的物理页存pgdir，每个页目录项会在page_insert时重新建立pte，这个pte指向的是原来的page，但是它的flags会变。原进程那个page的pte的flags也会相应调整。如果原进程的页不在内存中，就找到对应swap的页然后duplicate一个？

### pmm.h

#### kern-ucore/arch/i386/mm/pmm.h

- `page_ref`的四个函数变为了原子操作
  - rv64 porting：需要修改`atomic.h`中增加对应的原子操作。**TODO：变成真正的原子加减。**
- 增加了一系列多级页表的函数以及操作。

### swap.c

#### kern-ucore/arch/i386/mm/swap.c

只保留了check函数。

#### kern-ucore/mm/swap.c

使用的简化的LRU算法（LRU/2）。



### vmm.h

与arch无关，暂时不需要管。

#### kern-ucore/mm/vmm.h

- `VM_SHARE`的含义是当前vm_struct是否存在share memory。与ucore实现的简单COW不同，这里所有共享的memory都用了`shmem_struct`。

### vmm.c

#### kern-ucore/arch/i386/mm/vmm.c

只有`copy_from_user`、`copy_to_user`和`copy_string`。内容与ucore相同。

#### kern-ucore/mm/vmm.c

- 增加了对`mm`的`lock`操作函数。即`lock_mm`、`unlock_mm`、`try_lock_mm`

- 各种函数中由于增加了红黑树、share memory机制，会发生相应变化。也增加了很多函数。移植时需要根据该框架进行修改。

- `mm_destroy`中去掉了语句：

  ```c++
  mm = NULL;
  ```

  这是因为ucore中这段代码是不必要的。因为`mm`是个指针，而不是指针的引用or二级指针。




# Lab4 & Lab5

## 移植过程

lab4几乎不需要粘贴什么代码，但需要进行修改。

- `init_new_context`需要使用64bit的栈，因此需要考虑对齐的问题。
- ucore+改了`sys_exec`的代码，因此传参需要进行调整。
- vmm.c中需要增加`vm_flags`的修改。具体而言，需要在有`VM_WRITE`时将`perm`设置为`PTE_W|PTE_R`（参考lab2移植的记录）。此外，由于risc-v中有`PTE_X`，而x86没有考虑这一点，因此在`vm_flags`含有`VM_EXEC`时，需要将`perm`的设置增加`PTE_X`。



# Lab6

不需要移植。



# Lab7

不需要移植。

需要注意的是，原子操作需要保证地址是64bit对齐的，否则会报`AMO address misaligned`。



# Lab8

## 文件系统：

需要实现ide.c中关于device的实现。

各个硬件的实现途径：

- ARM实现：`ucore.ld.in`定义了`ramdisk`
- MIPS实现：定义了某个地址
- amd64实现：定义了一个`mods[0].start`地址，和硬件相关，有硬件相应的指令。
- i386实现：和硬件相关，有硬件相应的指令。

最终选择类似ARM的实现方式。自己定义了一套编译规则，并且还增加了`swapimg`，可以在编译期间指定。

## 移植过程

- user mode代码中`bool`变量是`int`，是4字节的，这对于risc-v的原子操作而言可能产生`AMO address misaligned`错误。将所有`bool`改成`long long`即可修复这个bug。
- 参考了ARM下关于`ramdisk_file`的`makefile`和`ld`文件，最终成功加入了`ide`驱动，并且可以在编译层面控制是否使用`ramdisk`。
- share memory有一个bug，会导致物理页映射错误，从而由于cpu是risc架构，怀疑正好映射到了某个io设备的物理地址，导致写的时候不会pg fault但是读的时候永远是0。已提交pull request。



# szx的ucore in risc-v 64bit 的bug不完全记录

- `print_pgdir`和`get_pgtable_items`是有bug的！因为没有考虑高级页表直接作为leaf table的情况。
- `map_pgdir`和`pmm_init`中对于pgdir的初始化都只用了`PDX0`，根据他的说法，“使用2级页表，来和32bit兼容”，我认为这是十分不妥的。我的移植中这些都是三级页表。
- 对`none leaf page table`设置了PTE_U，与手册的建议违背。