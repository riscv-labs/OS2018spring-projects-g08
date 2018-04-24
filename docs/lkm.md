内核可加载模块（LKM）学习笔记

# ucore+的LKM

* 整体仿照了Linux的设计
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
    * TODO:研究上面三个符号在LKM中应该怎么处理。我猜LKM在加载的时候可能还需要考虑内核接口提供的符号，使其能够链接上内核中的功能，而不应该指望LKM中包含完备的符号引用关系
    * 我发现如果输入的module不正确，`insmod`有时候会产生莫名的page fault


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
    * `NULL`
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