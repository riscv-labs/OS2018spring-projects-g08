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
* C中`__attribute__ ((section("name")))`可以把变量丢到指定的section里去（`PROGBITS`类型）而不是默认的`.data`或者`.bss`