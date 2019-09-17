#        《Linux内核分析与应用》第一章 : 概述 #
![](https://ss1.bdstatic.com/70cFvXSh_Q1YnxGkpoWK1HF6hhy/it/u=2356980406,1854346929&fm=26&gp=0.jpg)
## Are you ready? ##
> Linux内核像一座金山，又像一片茂密的森林，出发的入口在哪里？仅仅只有C基础和数据结构知识，也想感受Linux内核的魅力，可以上路么?

![](https://ss3.bdstatic.com/70cFv8Sh_Q1YnxGkpoWK1HF6hhy/it/u=1420018234,1150537973&fm=11&gp=0.jpg)

> 来吧！本章就是引领你感受Linux内核源代码的魅力!
## 初见内核 ##
> [下载源代码](https://www.kernel.org)或者进入[Linux内核在线源代码网站](https://elixir.bootlin.com/linux/latest/source)。更全面的文档，到这里来眺望一下[Linux内核文档](https://www.kernel.org/doc/html/latest/)
>
> ![](https://ss2.bdstatic.com/70cFvnSh_Q1YnxGkpoWK1HF6hhy/it/u=3686345393,3517005547&fm=26&gp=0.jpg)
## 第一章 ##
- **第一讲**　对Linux内核的整体结构了解后，你就像站在一座宫殿外面了，宫殿内的华丽必须进入并触摸才能感受
- **第二讲**　引领你触摸Linux内核源码，编写你有生以来的第一个操作系统级的程序-Linux内核模块

---
**到此，你终于与Linux内核有了亲密接触，而不仅仅是站在外面看看热闹了。本来热闹是别人的，但终于，你可以着手阅读源代码了，在源码中搜索[list_head](https://elixir.bootlin.com/linux/latest/source/tools/include/linux/types.h#L69)，进入它的源文件，查看[list.h](https://elixir.bootlin.com/linux/v5.1.6/source/include/linux/list.h#L489)的源文件。**

***
- **第三节 & 第四节**　引领你感受内核的双向链表和哈希表独特魅力，让你在短小精悍的一个个函数和宏中，颠覆你对双向链表和哈希表的认识，原来，双向链表是自带能量的，它不仅可以衍生出栈，还可以变出队列，至于形成一颗任意形状的树，对它来说也是轻而易举的，内核中的双链表和哈希表到底有多大魅力？不深入Linux内核源码，你对它的认识只限于第三四讲中提到的冰山一角。

![](https://ss1.bdstatic.com/70cFvXSh_Q1YnxGkpoWK1HF6hhy/it/u=1312805186,2700016073&fm=26&gp=0.jpg)

**心动不如行动,还是要动手实践**

- **第五讲**　引领你一步一步动手写自己的内核模块，因为手把手的引导，你曾经对编写内核代码的畏惧，就这样被一个一个字符打消掉了，原来，编写一个内核模块是一件难而又易的事。当你看到内核模块编译通过，插入到内核后，会不会有一种原来如此，不过如此，你不小心也踏入Linux内核的大门了，是的，你真的踏入Linux的这片汪洋大海了。但，且慢，如何把刚刚所学的双链表和哈希表应用起来呢？你在阅读了[list.h](https://elixir.bootlin.com/linux/v5.1.6/source/include/linux/list.h#L489)中的源代码后，还想把这些API应用起来，在内核模块中编写自己的链表和哈希表，你可以么，为甚不可以，行动吧，因为行动，前面的路逐渐变得宽阔起来。

![](https://ss0.bdstatic.com/70cFvHSh_Q1YnxGkpoWK1HF6hhy/it/u=586174726,552675386&fm=26&gp=0.jpg)


**如果你觉得自己还是搞不定一些内容，那就来[Linux内核之旅](http://www.kerneltravel.net)，或者关注[Linux内核之旅微信公众号](https://mp.weixin.qq.com/mp/qrcode?scene=10000005&size=102&__biz=MzI3NzA5MzUxNA==&mid=2664606528&idx=1&sn=61cc6ec4ff943db1b8d7384cc5d95247&send_time=)吧。**
***

**还觉得不过瘾?那就移步[陈老师谈内核](http://www.kerneltravel.net/?page_id=571)，有更多有趣的文章等着你。**
![](https://ss3.bdstatic.com/70cFv8Sh_Q1YnxGkpoWK1HF6hhy/it/u=1337866671,1936646298&fm=26&gp=0.jpg)

##### 更多学习资源 #####
　　 [Linux内核之旅网站](http://www.kerneltravel.net)

　　　　　　　　　　![](http://ww1.sinaimg.cn/large/005NFTS2ly1g72l6gix3hj30yn0j7tn2.jpg)

　　[Linux内核之旅微信公众号](https://mp.weixin.qq.com/mp/qrcode?scene=10000005&size=102&__biz=MzI3NzA5MzUxNA==&mid=2664606528&idx=1&sn=61cc6ec4ff943db1b8d7384cc5d95247&send_time=)

　　　　　　![](http://storage.xuetangx.com/discussion/user/8085113/discussion-20190601054755_20190601054755.jpg)