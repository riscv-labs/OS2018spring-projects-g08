系统编程相关笔记。

# GCC内置函数

* `__builtin_expect`：告知编译器表达式的期望值，方便编译器优化分支代码。Linux中定义了`likely`和`unlikely`两个宏，用来表示条件为真的可能性大小，使用的就是这个内置函数。
* `asmlinkage`：指定函数参数放在栈中
* `SYSCALL_DEFINEx`：Linux中定义的一个宏，用来定义系统调用。