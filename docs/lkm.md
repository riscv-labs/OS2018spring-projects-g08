可加载内核模块（LKM）学习笔记

# ucore+的LKM

* 整体仿照了Linux的设计
    * Linux下还提供了`modprobe`和`depmod`两个比较高层次的用户态工具
* 内核提供的接口：一系列系统调用
    * `SYS_list_module`
    * `SYS_init_module`
    * `SYS_cleanup_module`
* 用户态的相关工具：
    * `lsmod`：调用`SYS_list_module`
    * `insmod`
    * `rmmod`
* LKM编译：`src/kmod/`中的kernel module在x86下编译成功。编译时遇到的问题：
    * 显示`compiler-gcc-7.h`不存在。看了一下ucore+只提供了gcc-3和gcc-4的头文件，似乎是不支持gcc-7这么新的版本。解决方案是装了个gcc 4.9.4
    * 显示某个头文件中`__LINUX_ARM_ARCH__`未定义。解决方法是找到相应得文件随便定义一下（这个头文件似乎是ARM相关的，因此应该不会被用到，没什么关系）
* LKM文件中的section：

        There are 18 section headers, starting at offset 0x484:

        Section Headers:
        [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
        [ 0]                   NULL            00000000 000000 000000 00      0   0  0
        [ 1] .text             PROGBITS        00000000 000034 0000e0 00  AX  0   0  4
        [ 2] .rel.text         REL             00000000 000754 000080 08     16   1  4
        [ 3] .init.text        PROGBITS        00000000 000114 000028 00  AX  0   0  1
        [ 4] .rel.init.text    REL             00000000 0007d4 000020 08     16   3  4
        [ 5] .rodata           PROGBITS        00000000 00013c 000060 00   A  0   0  1
        [ 6] .modinfo          PROGBITS        00000000 00019c 00000c 00   A  0   0  4
        [ 7] .eh_frame         PROGBITS        00000000 0001a8 0000b8 00   A  0   0  4
        [ 8] .rel.eh_frame     REL             00000000 0007f4 000028 08     16   7  4
        [ 9] .data             PROGBITS        00000000 000260 00007c 00  WA  0   0 32
        [10] .rel.data         REL             00000000 00081c 000018 08     16   9  4
        [11] .gnu.linkonce.thi PROGBITS        00000000 0002e0 0000f4 00  WA  0   0 32
        [12] .rel.gnu.linkonce REL             00000000 000834 000008 08     16  11  4
        [13] .bss              NOBITS          00000000 0003d4 000000 00  WA  0   0  4
        [14] .comment          PROGBITS        00000000 0003d4 000024 01  MS  0   0  1
        [15] .shstrtab         STRTAB          00000000 0003f8 000089 00      0   0  1
        [16] .symtab           SYMTAB          00000000 00083c 0001b0 10     17  20  4
        [17] .strtab           STRTAB          00000000 0009ec 0000d4 00      0   0  1
        Key to Flags:
        W (write), A (alloc), X (execute), M (merge), S (strings)
        I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)
        O (extra OS processing required) o (OS specific), p (processor specific)

    TODO:在`mod.c`中我发现

        versindex = find_sec(hdr, sechdrs, secstrings, "__versions");
    
    但是上面的section表中并没有`__versions`这一项。

    * 其中`.modinfo` section只包含license信息：
        
          Hex dump of section '.modinfo':
            0x00000000 6c696365 6e73653d 47504c00          license=GPL.

    TODO:在`mod.c`中我发现

        staging = get_modinfo(sechdrs, infoindex, "staging");

    不知道这个`staging`字段是什么。我并没有在`.modinfo`中发现这个字段。

* 加载LKM
    * 在Makefile中修改sfsimg的生成规则，将`hello.ko`文件加入文件系统：
        
           @cp -r $(TOPDIR)/src/kmod/hello.ko $(TMPSFS)/lib/modules/
    
    * 尝试在ucore+中通过`insmod`加载`hello.ko`：`insmod hello`。发现：`simplify_symbols: Unknown symbol printk`等，即`printk,driver_register,bus_register`三个符号找不到。检查`hello.ko`的符号表发现其中这三个符号确实没有链接上任何值：

            23: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND printk
            24: 00000040    60 OBJECT  GLOBAL DEFAULT    9 test_bus_type
            25: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND driver_register
            26: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND bus_register
    * 将所有引用上述三个符号的语句全部注释掉后，`hello.ko`的符号表中不再包含它们。`hello.ko`能够正常加载和卸载了。加载后通过`lsmod`命令得到`Modules linked in: hello`，表明内核已经将`hello.ko`加入已加载模块列表中。
    * 研究上面三个符号在LKM中应该怎么处理。我猜LKM在加载的时候可能还需要考虑内核接口提供的符号，使其能够链接上内核中的功能，而不应该指望LKM中包含完备的符号引用关系
        *  分析Linux下的LKM `fat.ko`，发现其符号表中存在大量未完成解析的符号（包括`printk`：`317: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printk`），这意味着这些符号的解析确实是在LKM加载时才动态完成的（类似于动态链接）
        * http://tldp.org/HOWTO/Module-HOWTO/basekerncompat.html
        * ucore+在发现未解析的符号时确实会尝试对符号进行解析，但是为什么连`printk`这种符号都无法解析？
            * 内核中并没有`printk`这个函数。感觉情况是：ucore+的作者在写LKM时直接粘了Linux的代码，但是内核又有很大部分用的是与Linux不同的框架，这使这个LKM实际上并不兼容ucore+
    * 将原来`hello`中的`printk`换成`kprintf`，发现不会再有Unknown symbol的错误，LKM也能（表面上）加载、卸载，但是没有期望的输出。经过研究我发现：内核并没有在完成LKM的链接后执行`mod->init`，因为它是个空指针，而这个空指针意味着内核对LKM进行重定位时没能正确地修改`mod->init`。我发现重定位`mod->init`时修改的内存地址与`mod->init`的地址并不相等，于是我开始怀疑LKM中`module`结构体里`init`的位置与内核中定义的`module`中`init`的位置并不一样。我比较了`linux/module.h`（LKM使用）和`mod.h`（内核使用）中各自的`struct module`，发现它们的确不一样，`linux/module.h`中的`module`多出了很多字段。我将这些多余的字段注释掉后，`hello`就能在加载时正常输出字符串了。
    * 之后又发现卸载`hello`时`exit`为空指针。我又发现`linux/module.h`和`mod.h`中定义的`struct module`的更多不同。我通过将`exit`字段直接调整到`init`字段之后，确保了`exit`在二者的偏移量相同，使`hello`能够正常卸载并输出字符串。
    * 我发现如果输入的module不正确，`insmod`有时候会产生莫名的page fault
    * `linux/init.h`中，对`.gnu.linkonce.this_module`段中的`init_module`和`cleanup_module`的定义如下：

            /* Each module must use one module_init(). */
            #define module_init(initfn)					\
                static inline initcall_t __inittest(void)		\
                { return initfn; }					\
                int init_module(void) __attribute__((alias(#initfn)));
            // this directly set init_module=initfn

            /* This is only required if you want to be unloadable. */
            #define module_exit(exitfn)					\
                static inline exitcall_t __exittest(void)		\
                { return exitfn; }					\
                void cleanup_module(void) __attribute__((alias(#exitfn)));
        而在`hello.c`中：

            module_init(hello_init);
            module_exit(hello_exit);
        这使得`init_module`和`cleanup_module`这两个符号分别成为`hello_init`和`hello_exit`的代称。可以通过查看`hello.ko`的符号表看到：

            11: 00000029   143 FUNC    LOCAL  DEFAULT    1 hello_init
            12: 000000b8    24 FUNC    LOCAL  DEFAULT    1 hello_exit
            ...
            19: 00000029   143 FUNC    GLOBAL DEFAULT    1 init_module
            20: 000000b8    24 FUNC    GLOBAL DEFAULT    1 cleanup_module
        因此，在load LKM时，内核只需要调用`.modinfo`中的`init`和`exit`字段就可以让LKM完成初始化和退出时应该做的工作。

        TODO:`__inittest`和`__exittest`是什么作用？
* `insmod`：用户态的LKM加载工具
    * 检查dependency（TODO:实验）
        * 这个dependency是在一个文本文件中定义的。这个文件的每一行是一项规则，形如`mod: m1 m2 m3 ...`，表示`mod`依赖于`m1, m2, m3, ...`。需要注意`m1, m2, m3`需要按照其依赖顺序（拓扑排序）来排列。如果一个`mod`有多项规则，只有第一项是有效的
        * 匹配上这个文件中的规则后，按顺序加载依赖的`m1, m2, m3`
    * 加载module
        * 把`ko`文件内容全部读到内存缓冲区中
        * 系统调用`SYS_init_module`转到内核态完成加载（参数是缓冲区地址）
        * 释放内存缓冲区
* `SYS_init_module`
    * 将文件内容拷贝到内核地址（TODO:为什么不直接在内核中读文件？）
    * Sanity check：检查ELF magic、type（需要是relocatable）和shentsize，检查文件尾部是否被截断
    * 遍历section，找到几个比较特殊的section
        * 符号表
        * `.gnu.linkonce.this_module`
        * `__versions`
        * `.modinfo`
    * 重新对section的flags进行了一些设置：
        * `.modinfo`和`__versions`在执行时不保留（`ALLOC`置0）
        * 符号表和字符串表在执行时保留（`ALLOC`置1）
    * 检查了一下module的版本号、`staging`？（没有实现）
    * 检查module是否在已加载LKM列表中，如果是就返回报错
    * 设置module的状态（之后另外会详细讲）
    * `layout_sections`设置模块的core和init（相当于segment？）的大小、各自包含的section以及每个section在其中的位置（这个位置借用了`sh_entsize`字段）（非`ALLOC`的section不考虑）
        * （这么处理似乎是为了节省存储空间。只会在初始化时用到的代码和数据放在init中，在初始化结束后就可以释放）
    * 根据新的layout，生成module运行的内存镜像。core和init分开，各个section的数据按照排好的顺序放置（非`ALLOC`的section不管，`NOBITS`留空），重设`sh_addr`（section的地址，这时section headers都还在老位置）
    * 操作license（没实现）
    * 初始化module的`modules_which_use_me`链表
        * 这个链表用于维护module之间的符号引用关系
    * `simplify_symbols`进行符号解析。遍历符号表，发现`UNDEF`的符号就尝试解析（`resolve_symbol`）（TODO:实验）
        * `resolve_symbol -> find_symbol`，到内核原本的符号表及已经加载的LKM的符号表中找符号。这里的实现细节是：
            * 对于内核原本就有的符号表，使用符号`__start___ksymtab`和`__stop___ksymtab`记录其的首尾地址。在内核的连接器脚本中可以找到：
        
                    . = ALIGN(4);
                    PROVIDE(__start___ksymtab = .);
                    *(__ksymtab)
                    PROVIDE(__stop___ksymtab = .);  
            * 对于已加载LKM，直接用`struct module`中的信息就可以得到符号表的首尾地址
            * 使用回调函数的方式，对每个符号调用回调函数，回调函数比较符号名，以返回值表示是否继续进行符号表的遍历
            * 在找到匹配的符号后，将引用该符号的LKM加入到提供该符号的LKM（如果是个LKM的话）的`modules_which_use_me`列表中，将找到的符号值填到引用该符号的LKM的符号表中
    * 进行重定位（经过符号解析，符号的值已经确定。现在需要根据符号的值回修改引用符号位置的值）：
        * 找到重定位表sections(目前的实现不支持`RELA`类型)
        * 遍历重定位表的表项，找到符号表中对应的符号，根据重定位类型进行改写原来的值
    * 检查LKM导出的符号与现有的符号是否有命名冲突，如果有就报错（TODO:实验）
        * 导出的符号存在`__ksymtab`这个section中，只有导出的符号可以被其他LKM引用
    * 将LKM添加到已加载module列表中
    * 释放放置文件内容的内存空间
    * 执行`init`
    * 释放`init`空间
* `rmmod`：直接调用`SYS_cleanup_module`
* `SYS_cleanup_module`
    * 在开始卸载前检查几个事：
        * LKM的状态，它是不是已经死了
        * 是否有别的LKM引用了它的符号（`modules_which_use_me`是否为空）
        * 是否包含卸载函数（TODO:难道实际中会有无法卸载的LKM？）
    * 卸载：
        * 调用`exit`
        * 在`last_unloaded_module`中记录一下module名（TODO:有啥用？）
        * 从已加载module列表、其他LKM的`modules_which_use_me`列表中删除该module
        * 释放LKM占用的空间
* `lsmod`：直接调用`SYS_list_module`
* `SYS_list_module`：直接打印`modules`列表中所有module的名称

## 总结：数据结构

核心的数据结构是`struct module`，里面包含了module的基本信息：

* 状态。module有如下三种状态
    * `COMING`：正在加载中
    * `LIVING`：已经完成加载
    * `GOING`：正在卸载
* 符号：
    * 符号表`symtab`
    * 导出的、其他module可以引用的符号列表`syms`
* `modules_which_use_me`列表：所有引用了该module符号的module

当前系统中存在的module维护在`modules`链表中。

# ELF

学习LKM的时候发现对ELF的了解还不够全面，于是补了一下。

* 类型（`type`字段）
    * `REL`：relocatable
    * `EXEC`：executable
* Sanity check
    * `magic`
    * `type`
    * `machine`
    * `shentsize`
* section和segment（对应program headers）的区别？https://stackoverflow.com/questions/14361248/whats-the-difference-of-section-and-segment-in-elf-file-format
    * section主要用于链接过程。在可执行ELF中是可选的。有的section中的数据会被加入segment中，而有的section只为链接器提供元数据
    * segment是程序执行时加载的数据，只存在于链接后的ELF中。一个segment可以包含0个或多个section
* `readelf`
    * `-h`：显示ELF头信息
    * `-l`：显示segment信息
    * `-S`：显示section信息
    * `-s`：显示所有符号（主要读`.symtab`）
    * `-x`：hexdump指定的section
    * `-r`：显示重定位信息
* section类型

        #define SHT_NULL        0
        #define SHT_PROGBITS    1
        #define SHT_SYMTAB      2
        #define SHT_STRTAB      3
        #define SHT_RELA        4
        #define SHT_HASH        5
        #define SHT_DYNAMIC     6
        #define SHT_NOTE        7
        #define SHT_NOBITS      8
        #define SHT_REL         9
        #define SHT_SHLIB       10
        #define SHT_DYNSYM      11
    * `PROGBITS`：保存程序的数据。`.text`和`.data`是这种类型。
    * `NOBITS`：也是定义程序的数据，但数据不在文件中给出。`.bss`就是这种类型
    * `NULL`：0号section总是`NULL`，没有包含什么信息，只起哨兵的作用（类似于GDT中的0号段？）https://stackoverflow.com/questions/26812142/what-is-the-use-of-the-sht-null-section-in-elf
    * `STRTAB`：字符串表，包含一堆字符串
        * `.shstrtab`：包含各个section的名称。section的`name`字段指向这个section中的数据。ELF头部`shstrnd`字段指定了它的位置
        * `.strtab`：包含符号名称。`SYMTAB`会引用这个section中的字符串作为符号名（通过section的`link`字段）
        * `.stabstr`
    * `SYMTAB`：符号表
    * `RELA`：重定位表。其他section通过`info`字段引用。每一项包含需要重定位的地址在section中的偏移量、该地址指向的section的index及在此section中的偏移量`addend`。（重定位是以section为粒度的？）
    * `REL`：和`RELA`差不多，只是没有了`addend`（相当于`addend`为修改的地址，https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-54839.html）
            typedef struct {
                Elf32_Addr		r_offset;
                Elf32_Word		r_info;
            } Elf32_Rel;
            
            typedef struct {
                Elf32_Addr		r_offset;
                Elf32_Word		r_info;
                Elf32_Sword		r_addend;
            } Elf32_Rela;

        * `.rela.text`：`.text`使用的重定位表
* section的`address`字段：对于relocatable类型ELF，address都是0（似乎是的）
* C中`__attribute__ ((section("name")))`可以把变量丢到指定的section里去（`PROGBITS`类型）而不是默认的`.data`或者`.bss`

# RISC-V 64移植

* 使用RISCV工具链生成的LKM头部

        ELF 头：
        Magic：  7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
        类别:                              ELF64
        数据:                              2 补码，小端序 (little endian)
        版本:                              1 (current)
        OS/ABI:                            UNIX - System V
        ABI 版本:                          0
        类型:                              REL (可重定位文件)
        系统架构:                          RISC-V
        版本:                              0x1
        入口点地址：              0x0
        程序头起点：              0 (bytes into file)
        Start of section headers:          3696 (bytes into file)
        标志：             0x5, RVC, double-float ABI
        本头的大小：       64 (字节)
        程序头大小：       0 (字节)
        Number of program headers:         0
        节头大小：         64 (字节)
        节头数量：         16
        字符串表索引节头： 15
* 使用RISCV工具链生成的LKM的重定位信息：

        重定位节 '.rela.text' 位于偏移量 0x808 含有 52 个条目：
        偏移量          信息           类型           符号值        符号名称 + 加数
        000000000032  00200000001a R_RISCV_HI20      0000000000000068 test_bus_type + 0
        000000000032  000000000033 R_RISCV_RELAX                        0
        000000000036  00200000001b R_RISCV_LO12_I    0000000000000068 test_bus_type + 0
        000000000036  000000000033 R_RISCV_RELAX                        0
        000000000054  00110000001a R_RISCV_HI20      0000000000000030 .LC3 + 0
        000000000054  000000000033 R_RISCV_RELAX                        0
        000000000058  00110000001b R_RISCV_LO12_I    0000000000000030 .LC3 + 0
        000000000058  000000000033 R_RISCV_RELAX                        0
        00000000005c  002100000012 R_RISCV_CALL      0000000000000000 kprintf + 0
        00000000005c  000000000033 R_RISCV_RELAX                        0
        000000000064  000c00000012 R_RISCV_CALL      0000000000000000 test_bus_init + 0
        000000000064  000000000033 R_RISCV_RELAX                        0
        000000000078  00150000002c R_RISCV_RVC_BRANC 000000000000008c .L8 + 0
        00000000007a  00120000001a R_RISCV_HI20      0000000000000048 .LC4 + 0
        00000000007a  000000000033 R_RISCV_RELAX                        0
        00000000007e  00120000001b R_RISCV_LO12_I    0000000000000048 .LC4 + 0
        00000000007e  000000000033 R_RISCV_RELAX                        0
        000000000082  002100000012 R_RISCV_CALL      0000000000000000 kprintf + 0
        000000000082  000000000033 R_RISCV_RELAX                        0
        00000000008a  00160000002d R_RISCV_RVC_JUMP  000000000000009c .L9 + 0
        00000000008c  00130000001a R_RISCV_HI20      0000000000000058 .LC5 + 0
        00000000008c  000000000033 R_RISCV_RELAX                        0
        000000000090  00130000001b R_RISCV_LO12_I    0000000000000058 .LC5 + 0
        000000000090  000000000033 R_RISCV_RELAX                        0
        000000000094  002100000012 R_RISCV_CALL      0000000000000000 kprintf + 0
        000000000094  000000000033 R_RISCV_RELAX                        0
        00000000009c  000a0000001a R_RISCV_HI20      0000000000000000 test_driver + 0
        00000000009c  000000000033 R_RISCV_RELAX                        0
        0000000000a0  000a0000001b R_RISCV_LO12_I    0000000000000000 test_driver + 0
        0000000000a0  000000000033 R_RISCV_RELAX                        0
        0000000000a4  000d00000012 R_RISCV_CALL      0000000000000018 test_driver_register + 0
        0000000000a4  000000000033 R_RISCV_RELAX                        0
        0000000000b8  00170000002c R_RISCV_RVC_BRANC 00000000000000cc .L10 + 0
        0000000000ba  00120000001a R_RISCV_HI20      0000000000000048 .LC4 + 0
        0000000000ba  000000000033 R_RISCV_RELAX                        0
        0000000000be  00120000001b R_RISCV_LO12_I    0000000000000048 .LC4 + 0
        0000000000be  000000000033 R_RISCV_RELAX                        0
        0000000000c2  002100000012 R_RISCV_CALL      0000000000000000 kprintf + 0
        0000000000c2  000000000033 R_RISCV_RELAX                        0
        0000000000ca  00180000002d R_RISCV_RVC_JUMP  00000000000000dc .L11 + 0
        0000000000cc  00130000001a R_RISCV_HI20      0000000000000058 .LC5 + 0
        0000000000cc  000000000033 R_RISCV_RELAX                        0
        0000000000d0  00130000001b R_RISCV_LO12_I    0000000000000058 .LC5 + 0
        0000000000d0  000000000033 R_RISCV_RELAX                        0
        0000000000d4  002100000012 R_RISCV_CALL      0000000000000000 kprintf + 0
        0000000000d4  000000000033 R_RISCV_RELAX                        0
        0000000000f0  00140000001a R_RISCV_HI20      0000000000000068 .LC6 + 0
        0000000000f0  000000000033 R_RISCV_RELAX                        0
        0000000000f4  00140000001b R_RISCV_LO12_I    0000000000000068 .LC6 + 0
        0000000000f4  000000000033 R_RISCV_RELAX                        0
        0000000000f8  002100000012 R_RISCV_CALL      0000000000000000 kprintf + 0
        0000000000f8  000000000033 R_RISCV_RELAX                        0

        重定位节 '.rela.init.text' 位于偏移量 0xce8 含有 6 个条目：
        偏移量          信息           类型           符号值        符号名称 + 加数
        000000000008  001b0000001a R_RISCV_HI20      0000000000000020 .LC2 + 0
        000000000008  000000000033 R_RISCV_RELAX                        0
        00000000000c  001b0000001b R_RISCV_LO12_I    0000000000000020 .LC2 + 0
        00000000000c  000000000033 R_RISCV_RELAX                        0
        000000000010  002100000012 R_RISCV_CALL      0000000000000000 kprintf + 0
        000000000010  000000000033 R_RISCV_RELAX                        0

        重定位节 '.rela.data' 位于偏移量 0xd78 含有 3 个条目：
        偏移量          信息           类型           符号值        符号名称 + 加数
        000000000000  001900000002 R_RISCV_64        0000000000000000 .LC0 + 0
        000000000068  001a00000002 R_RISCV_64        0000000000000010 .LC1 + 0
        000000000088  000b00000002 R_RISCV_64        0000000000000000 test_bus_match + 0

        重定位节 '.rela.gnu.linkonce.this_module' 位于偏移量 0xdc0 含有 2 个条目：
        偏移量          信息           类型           符号值        符号名称 + 加数
        000000000068  001f00000002 R_RISCV_64        0000000000000046 init_module + 0
        000000000070  001e00000002 R_RISCV_64        00000000000000e8 cleanup_module + 0

* 目前实现的RISC-V重定位类型：
    * `R_RISCV_64`：直接将目标位置的64位值替换为符号值+addend
    * `R_RISCV_LO12_I`：I型指令中的立即数，取符号值+addend的低12位
    * `R_RISCV_LO12_S`：S型指令中的立即数，取符号值+addend的低12位
    * `R_RISCV_HI20`：`lui`中的立即数，取符号值+addend的高20位
    * `R_RISCV_CALL`：对应一个`auipc`和一个`jalr`（通过relaxation可能优化为一条指令，由于过程过于复杂，目前实现中未考虑）
    * `R_RISCV_RVC_JUMP`：压缩的jump指令。
    * `R_RISCV_RVC_BRANCH`：压缩的branch指令。
    * `R_RISCV_RELAX`：目前的实现是不管。也就是完全不考虑linker relaxation，不对代码进行压缩
* 重定位中的坑点：
    * 在RISC-V 64I中，`lui`和`auipc`会对得到的32位整数符号扩展得到64位整数，这个有时候是会造成麻烦的。例如在`R_RISCV_HI20`类型的重定位中，如果符号值+addend大于等于2^31，符号扩展后得到的结果就与预期不符。目前对这个问题的解决方法是在`R_RISCV_HI20`重定位中，使用`auipc`而不是`lui`指令
    * S型、I型指令以及`jalr`指令都会对立即数进行符号扩展。因此，重定位时不能简单将需要加载的值分成高20位和低12位，在低12位最高位为1时需要将高20位加1

# Linux的LKM

* 在SMP下，每个module有一个`percpu` section（`.data..percpu`），存放了per cpu数据。在`init_module`中内核会为每个CPU分配单独的空间放置该section的数据（`percpu` section会被分配空间，但是比较特殊，因此在Linux实现中是和其他section分开处理的）

    	/* We will do a special allocation for per-cpu sections later. */
	    info->sechdrs[info->index.pcpu].sh_flags &= ~(unsigned long)SHF_ALLOC; 

    内核的per cpu数据的起始地址和结束地址分别为`__per_cpu_start`及`__per_cpu_end`

        #define PERCPU_INPUT(cacheline)						\
        VMLINUX_SYMBOL(__per_cpu_start) = .;				\
        *(.data..percpu..first)						\
        . = ALIGN(PAGE_SIZE);						\
        *(.data..percpu..page_aligned)					\
        . = ALIGN(cacheline);						\
        *(.data..percpu..read_mostly)					\
        . = ALIGN(cacheline);						\
        *(.data..percpu)						\
        *(.data..percpu..shared_aligned)				\
        PERCPU_DECRYPTED_SECTION					\
        VMLINUX_SYMBOL(__per_cpu_end) = .;

    上面按照`cacheline`对齐似乎是为了避免并发访问per cpu时竞争cache的问题。

    per cpu数据的访问应当在禁用抢占的情况下进行。


* 每个module有个exception table（内核本身也有一个），用于存放fault时的跳转地址

    	. = ALIGN(4);
            __ex_table : AT(ADDR(__ex_table) - LOAD_OFFSET) {
                __start___ex_table = .;
                #ifdef CONFIG_MMU
                        *(__ex_table)
                #endif
                __stop___ex_table = .;
            }


# 其他

* `printk`：实际上是内核的日志机制。消息分为8个不同的级别（例如`KERN_INFO, KERN_ALERT`等）
* GCC的`__attribute__`：
    * `section("name")`：将变量放在指定的section
    * `alias("name")`：将一个符号设定为另一个符号的别名，即直接将一个符号的值（地址）赋给了另外一个符号
