RISC-V Privileged Architecture 1.10学习记录

# 指令集

* `misa`：包含指令集描述信息
    * 高2位：log(XLEN) - 4
    * 低26位：指令集扩展（以26个英文字母表示）
        * D：双精度浮点
        * F：单精度浮点
        * A：原子指令扩展
        * N：用户级中断
        * Q：四倍精度浮点
        * U：用户级实现
        * S：Supervisor级实现
* `mstatus`：包含处理器状态（包括一些协处理器）
* `mcycle`、`minstret`、`mhpmcountern`：一堆计数器。可以使用`mcounteren`控制supervisor和user能否读取它们，用`scounteren`控制user能否读取它们


# 中断/异常/系统调用

中断分为三类：
* 软件中断：`ip`中的`xSIP`位被软件置1（IPI实现方式）
* 时钟中断：M-mode的由硬件置（？），其余由M-mode软件置1。（`xTIP`）
* 外部中断：同上（`xEIP`）

中断的使能
* 影响所有类型中断的使能：`status`中的`SIE`, `UIE`（还有`SPIE`, `UPIE`）
* 部分类型中断的使能：`SEIE`, `STIE`, `SSIE`, `UEIE`等

处理：
* 异常处理返回：`xret`指令，会从相应的栈中弹出数据，并且将`pc`指向`xepc`
* 中断向量：`xtvec`寄存器，支持两种标准的模式
    * 0：所有的异常都在`BASE`处理
    * 1：所有同步异常在`BASE`处理，所有异步异常（中断）在`BASE + 4 * cause`处处理
* 代理：`xedeleg`和`xideleg`可以将指定类型的中断代理给更低的一级模式进行处理（感觉有点像高级语言异常处理里把异常抛出的操作）
* trapframe：保存在`xstatus`寄存器中，其中`xpie`是中断前的中断使能位（相当于x86的`IF`位），`xepc`就是`EPC`，`xpp`是中断前的特权级。


等待中断：`wfi`指令，告知CPU暂停内核线程等待中断。


# 访存

* 宽松内存模型：一个线程中的访存操作在另一个线程中的顺序可能不一致。`fence`指令保证了其两侧的先后关系在其他线程中也能保持一致

## 分页/地址转换与保护（ATP）

* `satp`：保存页目录表的页帧号、地址空间表示（ASID）、ATP模式（包括无保护以及几种页式转换）
