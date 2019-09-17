# 大碰撞！当Linux多线程遭遇Linux多进程

```
作者简介：
廖威雄，目前就职于珠海全志科技股份有限公司从事linux嵌入式系统(Tina Linux)的开发，主要负责文件系统和存储的开发和维护，兼顾linux测试系统的设计和持续集成的维护。
拆书帮珠海百岛分舵的组织长老，二级拆书家，热爱学习，热爱分享。
```

## 背景 

本文并不是介绍Linux多进程多线程编程的科普文，如果希望系统学习Linux编程，可以看[《Unix环境高级编程》第3版](https://book.douban.com/subject/1788421/)

本文是描述多进程多线程编程中遇到过的一个坑，并从内核角度分析其原理。这里说的多进程多线程并不是单一的**多进程或多线程**，而是**多进程和多线程**，往往会在写一个大型应用时才会用到多进程多线程的模型。

这是怎么样的一个坑呢？假设有下面的代码：


![](http://wx1.sinaimg.cn/mw690/005NFTS2ly1g6bywupbtyj30gz0i1q99.jpg)


 童鞋们能分析出来，线程函数```sub_pthread```会被执行多少次么？线程函数打印出来的ID是父进程ID呢？还是子进程ID？还是父子进程都有？

答案是，只会执行1次，且是父进程的ID！为什么呢？

``````
[GMPY@10:02 share]$./signal-safe 
ID 6889: in sub_pthread
ID 6889 (father)
ID 6891 (children)
``````

裤子都脱了，你就给我看这个？当然，这个没什么悬念，到目前为止还很简单。精彩的地方正式开始。



## 线程和fork

***在已经创建了多线程的进程中调用fork创建子进程，稍不注意就会陷入死锁的尴尬局面***

以下面的代码做个例子：

![](http://wx2.sinaimg.cn/mw690/005NFTS2ly1g6bywv6s7gj30gu0bldiv.jpg)
![](http://wx1.sinaimg.cn/mw690/005NFTS2ly1g6bywvqguyj30fa0i8te6.jpg)

执行效果如下：

```
[GMPY@10:37 share]$./test 
--- sub thread lock ---
children burn
--- sub thread unlock ---
--- father lock ---
--- father unlock ---
--- sub thread lock ---
--- father lock ---
--- sub thread unlock ---
--- father unlock ---
--- sub thread lock ---
--- sub thread unlock ---
--- father lock ---
```

我们发现，子进程挂了，在打印了```children burn```后，没有了下文，因为在**子进程获取锁的时候，死锁了！**

凭什么啊？```sub_pthread```线程不是有释放锁么？父进程都能在线程释放后获取到锁，为什么子线程就获取不到锁呢？

在《Unix环境高级编程 第3版》的12.9章节中是这么描述的：

    子进程通过继承整个地址空间的副本，还从父进程那儿继承了每个互斥量、读写锁和条件变量的状态。
    如果父进程包含一个以上的线程，子进程在fork返回以后，如果紧接着不是马上调用exec的话，就需要清理锁状态。
    在子进程内部，只存在一个线程，它是由父进程中调用fork的线程的副本构成的。
    如果父进程中的线程占有锁，子进程将同样占有这些锁。
    问题是子进程并不包含占有锁的线程的副本，所以子进程没有办法知道它占有了哪些锁、需要释放哪些锁。
    ......
    在多线程的进程中，为了避免不一致状态的问题，POSIX.1声明，在fork返回和子进程调用其中一个exec函数之间，
    子进程只能调用异步信号安全的函数。这就限制了在调用exec之前子进程能做什么，但不涉及子进程中锁状态的问题。

究其原因，就是子进程成孤家寡人了。

每个进程都有一个主线程，这个线程参与到任务调度，而不是进程，[可以参考文章](https://www.cnblogs.com/gmpy/p/10265284.html)。

![](http://wx2.sinaimg.cn/mw690/005NFTS2ly1g6bywwu3scj308r08a3z5.jpg)

 在上面的例子中，父进程通过```pthread_create```创建出了一个小弟```sub_pthread```，父进程与小弟之间配合默契，你释放锁我就获取，玩得不亦乐乎。

![](http://wx4.sinaimg.cn/mw690/005NFTS2ly1g6bywxd1sej308n0gi0u9.jpg)

 这时候，父进程生娃娃了，这个新生娃娃**集成了父进程的绝大部分资源，包括了锁的状态**，然而，子进程并没有共生出小弟，就是说**子进程并没同时创建出小弟线程**，他就是一个坐拥金山的孤家寡人。

所以，问题就来了。如果在父进程创建子进程的时候，父进程的锁被小弟```sub_pthread```占用了，```fork```生出来的子进程锁的状态跟父进程一样一样的，锁上了！被人占有了！因此子进程再获取锁就死锁了。

或者你会说，我在fork前获取锁，在fork后再释放锁不就好了？是的，能解决这个问题，我们自己创建的锁，所以我们知道有什么锁。

最惨的是什么呢？你根本无法知道你调用的函数是否有锁。例如常用的```printf```，其内部实现是有获取锁的，因此在fork出来的子进程执行exec之前，**甚至都不能调用printf**。

我们看看下面的示例：

![](http://wx1.sinaimg.cn/mw690/005NFTS2ly1g6bywxwvhhj30ex0ibgqv.jpg)

 上面的代码主要做了两件事：

1. 创建线程，循环printf打印字符'\r'
2. 循环创建进程，在子进程中调用printf打印字串

由于printf的锁不可控，为了加大死锁的概率，为```fork```套了一层循环。执行结果怎么样呢？

```
root@TinaLinux:/mnt/UDISK# demo-c 
fork
ID 1684: in sub_pthread
ID 1684 (father)
ID 1686 (children)
ID 1686 (children) exit
fork
ID 1684 (father)
ID 1687 (children)
ID 1687 (children) exit
fork
ID 1684 (father)
```

结果在第3次```fork```循环的时候陷入了死锁，子进程不打印不退出，导致父进程```wait```一直阻塞。

*上面的结果在全志嵌入式Tina Linux平台验证，比较有意思的是，同样的代码在PC上却很难复现，可能是C库的差异引起的*

**在fork的子进程到exec之间，只能调用异步信号安全的函数**，这异步信号安全的函数就是认证过不会造成死锁的！

异步信号安全不再展开讨论，有问题找男人

> man 7 signal

 检索关键字```Async-signal-safe functions```

## 内核原理分析

我们知道，Linux内核中，用```task_struct```表示一个进程/线程，嗯，换句话说，**不管是进程还是线程，在Linux内核中都是用```task_struct```的结构体表示**。

关于进程与线程的异同，可以看文章[《线程调度为什么比进程调度更少开销？》](https://www.cnblogs.com/gmpy/p/10265284.html)，这里不累述。

按这个结论，我们```pthread_create```创建小弟线程时，内核实际上是copy父进程的```task_struct```，创建小弟线程的```task_struct```，且让小弟```task_struct```与父进程```task_struct```共享同一套资源。

如下图

![](http://wx3.sinaimg.cn/mw690/005NFTS2ly1g6bywyi1wyj30a4070glt.jpg)

 在父进程```pthread_create```之后，父进程和小弟线程组成了我们*概念上的父进程*。什么是概念上的父进程呢？在我们的理解中，创建的线程也是归属于父进程，这是概念上的父进程集合体，然而在Linux中，父进程和线程是独立的个体，他们有自己的调度，有自己的流程，就好像一个屋子下不同的人。

父进程fork过程，发生了什么？

跟进**系统调用fork**的代码：

![](http://wx2.sinaimg.cn/mw690/005NFTS2ly1g6bywzz5djj30li0g30yw.jpg)

 嗯...只是copy了```task_struct```，怪不得fork之后，子进程没有伴生小弟线程。所以fork之后，如下图：

![](http://wx3.sinaimg.cn/mw690/005NFTS2ly1g6byx0ed7jj308709lwev.jpg)

*（为了方便理解，下图忽略了Linux的写时copy机制）*

 Linux如此```fork```，这与锁有什么关系呢？我们看下内核中对互斥锁的定义：

![](http://wx1.sinaimg.cn/mw690/005NFTS2ly1g6bywyvd5fj30mo04pdhj.jpg)

 一句话概述，就是 **通过原子变量标识和记录锁状态**，用户空间也是一样的做法。

变量值终究是保存在内存中的，不管是保存在堆还是栈亦或其他，终究是(虚拟)内存中某一个地址存储的值。

结合Linux内核的```fork```流程，我们用这样一张图描述进程/线程与锁的关系：

![](http://wx3.sinaimg.cn/mw690/005NFTS2ly1g6bywz9btnj30dw0f3q4d.jpg)

（完）