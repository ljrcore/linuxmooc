# Linux内核之浅谈内存寻址
## 前言
最近在看内存寻址的内容，略有所得，发此文与大家一起交流。我们知道计算机是由硬件和软件组成，硬件主要包括运算器、控制器、存储器、输入设备和输出设备，软件主要是操作系统和用户应用软件，其中操作系统是联系硬件和软件的桥梁。本文主要分享运算器关于内存寻址的重点内容，从内存寻址的硬件机制重点和内核代码动手实践两部分来分享，欢迎交流，文中若有错误之处，还请指出。

# 一、内存寻址硬件机制
## 1、内存寻址
计算机在访问内存的时候，一般我们用眼睛能看到的地址都是**虚拟地址**，而内存条上每个内存单元的实际地址就是**物理地址**，那我们是如何访问到计算机内存条上的物理地址的呢？这就要用到地址转换，x86以上的CPU转换地址过程如下：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190711143224638.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

MMU是内存管理单元，它和CPU在一起，是专门来支持虚拟内存管理的，32位以上的处理器才会有，它的作用就是把虚拟地址转换为物理地址。CPU把程序编译链接后形成的虚拟地址送给MMU，MMU将此虚拟地址转换成物理地址送给存储器，操作系统配合MMU把虚拟地址转换为物理地址。此过程中可分为两个阶段，分别引入了**分段机制**和**分页机制**，第一阶段是用分段机制把二维的虚拟地址转换为线性地址，第二阶段是用分页机制是把线性地址转换为物理地址，此处所说的**线性地址**是一段连续的，不分段的，范围为0-4GB的地址空间，MMU地址转换过程示意图如下图。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190711144526313.png)

## 2、分段机制
分段机制就是为了把虚拟地址空间的一个地址转换为线性地址空间的一个线性地址，其转换关系如下图所示的段描述符表（段表）来描述。

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190711145303566.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

- 段号：描述的是虚拟地址空间段的编号
- 基地址：是线性地址空间段的起始地址
- 界限：在虚拟地址空间中，段内可以使用的最大偏移量。
- 属性：表示段的特性。例如，该段是否可被读出或写入，或者该段是否作为一个程序来执行，以及段的特权级等等。

虚拟地址到线性地址的转换方法：**线性地址=段的起始地址+偏移量**

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190711145107795.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

> 保护模式下的其他描述符表:

- 全局描述符表GDT（Gloabal Descriptor Table）
- 中断描述符表IDT（Interrupt Descriptor Table） 
- 局部描述符表LDT（Local Descriptor Table）

### （1）段选择符
段选择符（段选择子）是段的一个十六位标志符，如下图所示。段选择符并不直接指向段，而是指向段表中定义段的段号。 段选择符包括 3 个字段的内容： 

- **RPL表示请求者的特权级（Requestor Privilege Level）**

  保护模式提供了四个特权级，用0~3四个数字表示 ，很多操作系统（如Linux,Windwos）只使用了其中的最低和最高两个，即0表示最高特权级，对应内核态；3表示最低特权级，对应用户态。保护模式规定，高特权级可以访问低特权级，而低特权级不能随便访问高特权级。

- **TI（Table Index）**

  TI = 0 ，表示描述符在GDT中，TI = 1，表示描述符在LDT中
  
- **索引值**

  给出了描述符在GDT或LDT表中的索引项号。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190717205144730.png)

### （2）段描述符
每个段描述符长度是 8 字节，含有三个主要字段：段基地址、段限长和段属性，其结构如下图。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190717212159257.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

**定义段描述符的相关代码见Linux内核源码**

> /arch/x86/include/asm/segment.h

这个头文件定义了一些访问CPU段寄存器或与段寄存器有关的内存操作函数，在Linux操作系统中，当用户通过系统调用开始执行内核代码时，内核程序会首先在段寄存器DS和ES中加载全局描述符表GDT中的内核数据段描述符。

```c
/* Constructor for a conventional segment GDT (or LDT) entry */
/* This is a macro so it can be used in initializers */
#define GDT_ENTRY(flags, base, limit)           \
     ((((base)  & _AC(0xff000000,ULL)) << (56-24)) | \
     (((flags) & _AC(0x0000f0ff,ULL)) << 40) |  \
     (((limit) & _AC(0x000f0000,ULL)) << (48-16)) | \
     (((base)  & _AC(0x00ffffff,ULL)) << 16) |  \
     (((limit) & _AC(0x0000ffff,ULL))))
```

## 3、分页机制
分页在分段之后进行，是继段机制把虚拟地址转换为线性地址后，进一步把该线性地址再转换为物理地址。
- 是什么？实际上分页也就是就是将线性地址空间划分成若干大小相等的片，称为页。
- 为什么？分页是为了让每个进程可以拥有自己独立的虚拟内存空间。
- 怎么做？映射函数：Pa=f(va)
  - 时间的优化。因为访存很频繁，因此，映射函数f一定要简单，否则会效率很低，所以需要简单查表算法，这也就是页表引入的原因。
   - 空间的优化。因为内存空间是按字节编址的，地址一一进行映射的话，效率也很低，于是要按照一定的粒度（也就是页）进行映射，这样，粒度内的相对地址（也就是页内偏移量）在映射时保持不变。
  
>  线性地址到物理地址的转换过程描述如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190712163403402.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

一个线性地址由10位目录表+10位页表+12位偏移量组成，当给定一个线性地址时：

- 第一步，用最高10位作为页目录项的索引，将它乘以4，与CR3中的页目录的起始地址相加，获得相应目录项在内存的地址。
- 第二步，从这个地址开始读取32位页目录项，取出其高20位，再给低12位补0，形成页表在内存的起始地址。
- 第三步，用中间的10位作为页表中页表项的索引，将它乘以4，与页表的起始地址相加，获得相应页表项在内存的地址。
- 第四步，从这个地址开始读取32位页表项，取出其高20位，再将线性地址的第11~0位放在低12位，形成最终32位页面物理地址。 

### Linux中的分页

Linux主要采用分页机制来实现虚拟存储器管理，这是因为以下两个原因: 
- Linux巧妙地绕过了段机制（线性地址=偏移量）
- Linux设计目标之一就是具有可移植性，但很多CPU并不支持段。

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190714084454938.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

目前许多处理器都采用64位结构的，为了保持可移植性，Linux目前采用四级分页模式，为此，定义了四种类型的页表：

- 页全局目录PGD（Page Global Directory）
- 页上级目录PUD（Page Upper Directory）
- 页中间目录PMD（Page Middle Derectory）
- 页表PT（Page table）

页全局目录PGD包含若干页上级目录PUD的地址， 页上级目录PUD又依次包含若干页中间目录PMD的地址， 页中间目录又包含若干页表PT的地址， 每一个页表项指向一个页框。  因此线性地址因此被分成五个部分，而每一部分的大小与具体的计算机体系结构有关。

 **页表的相关代码见Linux内核源码**

> include/asm-generic/pgtable-nopud.h
> include/asm-generic/pgtable-nopmd.h
> arch/x86/include/asm/pgtable-2level*.h
> arch/x86/include/asm/pgtable-3level*.h
> arch/x86/include/asm/pgtable_64*.h
> arch/x86/include/asm/pgtable_64_types.h

```c

#ifndef __ASSEMBLY__
#include <linux/types.h>
/*
 * These are used to make use of C type-checking..
 */
typedef unsigned long	pteval_t;
typedef unsigned long	pmdval_t;
typedef unsigned long	pudval_t;
typedef unsigned long	pgdval_t;
typedef unsigned long	pgprotval_t;
typedef struct { pteval_t pte; } pte_t;   	
#endif	/* !__ASSEMBLY__ */## 

```

## 4、保护模式
### （1）实模式
实模式下存储器地址的分段允许的最大寻址空间为1MB（因为8086/8088地址总线宽度是20为 2^20=1048576=1024k=1M），其他的微处理器也为1M， 实际上实模式就是为8086/8088而设计的工作方式，它要解决在16位字长的机器里怎么提供20位地址的问题，而解决的方法是采用存储器地址分段的方法。

从0地址，每16个字节为一小段，而在1MB存储器里每个储存单元都有一个唯一的20位物理地址，便于CPU访问存储器，所以这个20位物理地址由16位段地址和16位偏移地址组成，把段地址（因为是首地址，所以低四位全为0，只取高16位）左移4位再加上偏移地址值就形成物理地址，即**物理地址=段地址+偏移地址**。
### （2）保护模式 
x86处理器地址总线位数增加到24位，可以访问16M地址空间。于是引入保护模式，这种模式下，内存段的访问受到了限制。访问内存时不能直接从段寄存器获得段起始地址了，而要经过额外转换和检查。为与过去兼容，80286内存寻址有两种方式：保护模式和实模式。系统启动时处理器处于实模式，只能访问1M内存空间，经过处理可以进入保护模式，可访问16M内存空间，但要从保护模式回到实模式必须重启机器。由于实模式只提供了1MB的寻址空间，不够用，而且随着多任务出现对寻址空间的要求越来越高，如80826就提供了16MB，80836就提供了达4GB的地址空间，而且虚拟存储器也能扩展空间，而保护模式寻址则对虚拟存储特性有很好的支持。

**保护有两层含义：**
 - 任务间保护：多任务操作系统中，一个任务不能破坏另一个任务的代码，这是通过内存分页以及不同任务的内存页映射到不同物理内存上来实现的。 
 - 任务内保护：系统代码与应用程序代码虽处于同一地址空间，但系统代码具有高优先级，应用程序代码处于低优先级，规定只能高优先级代码访问低优先级代码，这样杜绝用户代码破坏系统代码。

# 二、内存寻址内核代码实践
## 1、程序源码

下面的代码主要功能是在内核中先申请一个页面，然后利用内核提供的函数按照寻页的步骤一步步查询各级页目录，最终找到所对应的物理地址。具体过程为首先根据pid我们可以得到进程的task_struct，进而通过task_struct得到mm，通过mm和虚拟地址得到pgd，通过pgd和虚拟地址得到p4d，通过p4d和虚拟地址得到pud，通过pud和虚拟地址得到pmd，通过pmd和虚拟地址得到pte，有了页表pte我们就可以计算物理地址了，页框的物理地址 page_addr = pte_val(*pte) & PAGE_MASK，页偏移地址page_offset = vaddr & ~PAGE_MASK，最终要求的物理地址paddr = page_addr | page_offset。

###  必要的头文件

```c
#include <linux/init.h>//包含了模块的初始化的宏定义及一些其他函数的初始化函数
#include <linux/module.h>//内核模块必备头文件
#include <linux/mm.h>//// 内存管理相关头文件，含有页面大小定义和一些页面释放函数原型。
#include <linux/mm_types.h>//内存管理相关头文件
#include <linux/sched.h>//进程调度相关头文件
#include <linux/export.h>//
#include <linux/delay.h>//延时函数头文件
//定义全局变量
static unsigned long cr0,cr3;//定义CR0和CR3
static unsigned long vaddr = 0;//定义虚拟地址的全局变量
```

### 打印页机制中的一些重要参数
```c
static void get_pgtable_macro(void)
{
	cr0 = read_cr0();//获得CR0寄存器的值 
	cr3 = read_cr3_pa();//获得CR3寄存器的值 
	printk("cr0 = 0x%lx, cr3 = 0x%lx\n",cr0,cr3);//打印CR0和CR3的值
	//_SHIFT宏用来描述线性地址中相应字段所能映射区域大小的位数
	printk("PGDIR_SHIFT = %d\n", PGDIR_SHIFT);//打印页全局目录项能映射的区域大小的位数
	printk("P4D_SHIFT = %d\n",P4D_SHIFT);//打印P4D目录项能映射的区域大小的位数
	printk("PUD_SHIFT = %d\n", PUD_SHIFT);//打印页上级目录项能映射的区域大小的位数
	printk("PMD_SHIFT = %d\n", PMD_SHIFT);//打印页中间目录项可以映射的区域大小的位数
	printk("PAGE_SHIFT = %d\n", PAGE_SHIFT);//打印page_offset字段所能映射区域大小的位数
	//指示相应页目录表中项的个数
	printk("PTRS_PER_PGD = %d\n", PTRS_PER_PGD);//打印页全局目录项数
	printk("PTRS_PER_P4D = %d\n", PTRS_PER_P4D);//打印P4D目录项数
	printk("PTRS_PER_PUD = %d\n", PTRS_PER_PUD);//打印页上级目录项数
	printk("PTRS_PER_PMD = %d\n", PTRS_PER_PMD);//打印页中级目录项数
	printk("PTRS_PER_PTE = %d\n", PTRS_PER_PTE);//打印页表项数
	printk("PAGE_MASK = 0x%lx\n", PAGE_MASK);//页内偏移掩码，屏蔽page_offset字段
}
```

### 线性地址转换为物理地址
```c
static unsigned long vaddr2paddr(unsigned long vaddr)
{
    //创建变量保存页目录项
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long paddr = 0;
    unsigned long page_addr = 0;
    unsigned long page_offset = 0;
    //获取页全局目录PGD，第一个参数当前进程的mm_struct，所有进程共享一个内核页表
    pgd = pgd_offset(current->mm,vaddr);//获得pgd的地址
    printk("pgd_val = 0x%lx, pgd_index = %lu\n", pgd_val(*pgd),pgd_index(vaddr));//打印pgd地址和索引
    if (pgd_none(*pgd))//判断pgd页表项是否为空
	{
        printk("not mapped in pgd\n");
        return -1;
    }
    //获取P4D，新的Intel芯片的MMU硬件规定可以进行5级页表管理，内核在PGD和PUD之间，增加了一个叫P4D的页目录
    p4d = p4d_offset(pgd, vaddr);//获得p4d的地址
    printk("p4d_val = 0x%lx, p4d_index = %lu\n", p4d_val(*p4d),p4d_index(vaddr));//打印p4d地址和索引
    if(p4d_none(*p4d))//判断p4d页表项是否为空
    { 
        printk("not mapped in p4d\n");
        return -1;
    }
    //获取页上级目录PUD
    pud = pud_offset(p4d, vaddr);//获得pud的地址
    printk("pud_val = 0x%lx, pud_index = %lu\n", pud_val(*pud),pud_index(vaddr));//打印pud地址和索引
    if (pud_none(*pud)) //判断pud页表项是否为空
	{
        printk("not mapped in pud\n");
        return -1;
    }
    //获取页中间目录PMD 
    pmd = pmd_offset(pud, vaddr);获得pmd的地址
    printk("pmd_val = 0x%lx, pmd_index = %lu\n", pmd_val(*pmd),pmd_index(vaddr));//打印pmd地址和索引
    if (pmd_none(*pmd)) ////判断pmd页表项是否为空
	{
        printk("not mapped in pmd\n");
        return -1;
    }
    pte = pte_offset_kernel(pmd, vaddr);//获得pte的地址
    printk("pte_val = 0x%lx, ptd_index = %lu\n", pte_val(*pte),pte_index(vaddr));//打印pte地址和索引
    if (pte_none(*pte)) //判断pte页表项是否为空
	{
        printk("not mapped in pte\n");
        return -1;
    }
    page_addr = pte_val(*pte) & PAGE_MASK;//获得页框的物理地址
    page_offset = vaddr & ~PAGE_MASK;//获得页偏移地址
    paddr = page_addr | page_offset;//获得物理地址
    printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
    printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);//打印虚拟地址和转换后的物理地址
    return paddr;
}
```

### 加载内核模块
```c	
static int __init v2p_init(void)
{
    unsigned long vaddr = 0 ;
    printk("vaddr to paddr module is running..\n");
    get_pgtable_macro();//打印主要参数
    printk("\n");
    vaddr = __get_free_page(GFP_KERNEL);//在内核ZONE_NORMAL中申请一块页面
    if (vaddr == 0) 
	{
        printk("__get_free_page failed..\n");
        return 0;
    }
    sprintf((char *)vaddr, "hello world from kernel");
    printk("get_page_vaddr=0x%lx\n", vaddr);
    vaddr2paddr(vaddr);//调用线性地址转换物理地址的函数
    ssleep(600);//延时
    return 0;
}
```

### 卸载内核模块
```c
static void __exit v2p_exit(void)
{
    printk("vaddr to paddr module is leaving..\n");
    free_page(vaddr);
}
```
### 入口、出口，许可证
```c
module_init(v2p_init);//内核入口函数
module_exit(v2p_exit);//内核出口函数
MODULE_LICENSE("GPL"); //许可证
```
## 2、Makefile
本程序Makefile文件代码如下
```shell
#产生目标文件
obj-m:=v2p.o
#路径变量，指明当前路径
CURRENT_PATH:=$(shell pwd)
#指明内核版本号
LINUX_KERNEL:=$(shell uname -r)
#指明内核源码的绝对路径
LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)
#编译模块
all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
#清理模块
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
```
**插播一条Makefile小知识**

- obj-m 意思是将后面的内容编译成内核模块
- obj-y 编译进内核
- obj-n 不编译

## 3、编译加载模块
### （1）编译模块
使用命令make后，生成如下文件
![](https://img-blog.csdnimg.cn/20190711115021660.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

### （2）加载模块
使用命令

    sudo insmod v2p.ko

将v2p.ko加载到内核中
### （3）查看模块
使用命令

	lsmod

查看系统已插入的内核模块，如下图模块已经加载到内核中
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190711120357179.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

## 4、查看结果
使用命令

	dmesg

查看系统日志，结果如下图所示

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190712164902557.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0MjU4MzQ0,size_16,color_FFFFFF,t_70)

从图中可以看到CR0、CR3寄存器的值，PGDIR、PUD、P4D、PMD、PAGE目录offset字段的位数、目录项数、地址和索引等，虚拟地址vaddr = ffff95590e2f4000, 转换后的物理地址paddr = 800000000e2f4000。