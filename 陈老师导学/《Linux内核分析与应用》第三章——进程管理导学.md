# 《Linux内核分析与应用》第三章 : 进程管理 #
> 你认识进程么，就相当于问你认识自己一样难于回答，因为进程每一瞬间都是变化的，就像你的思想无时无刻不在变化一样，因此，本章对进程的讲解可以说只是一种归纳和总结
## 第三章 ##
- **第一讲**　对进程最基本的东西进行介绍
- **第二讲**　直奔进程的创建，创建一个进程像生一个孩子一样，实在不是一件容易的事，本讲进行归纳后给出简明扼要的概述
- **第三讲**　作为重点的调度，也只是给出了一种一般性的入门级介绍。了解了这些以后，你可以动手实践了么，当然可以
- **第四讲**　动手实践，让你对进程的PCB，也就是task_struct结构中的字段可以拽出来看看，认识它的本来面目，有了这些基本的知识后，你是否就可以与企业一线的工程实战对接了呢
- **第五讲**　内核专家谢宝友老师给大家的一份礼物，你务必亲手接住，并把他抛给大家的一个一个问题给化解了，那么，至少，你对进程有了初步的了解

**且慢，进程背后琳琅满目的宝贝到哪里挖？这里列出一份CSDN上Linux进程的管理和调度的清单:**
## 1. 进程的描述 ##
- [ Linux进程描述符task_struct结构体详解--Linux进程的管理与调度（一）](https://blog.csdn.net/gatieme/article/details/51383272)
- [ Linux的命名空间详解--Linux进程的管理与调度（二）](https://blog.csdn.net/gatieme/article/details/51383322)
- [Linux进程ID号--Linux进程的管理与调度（三）](https://blog.csdn.net/gatieme/article/details/51383377)
## 2. 进程的创建 ##
- [Linux下的进程类别（内核线程、轻量级进程和用户进程）以及其创建方式--Linux进程的管理与调度（四）](https://blog.csdn.net/gatieme/article/details/51482122)
- [Linux下0号进程的前世(init_task进程)今生(idle进程)—-Linux进程的管理与调度（五）](http://blog.csdn.net/gatieme/article/details/51484562)
- [Linux下1号进程的前世(kernel_init)今生(init进程)—-Linux进程的管理与调度（六）](http://blog.csdn.net/gatieme/article/details/51532804)
- [Linux下2号进程的kthreadd–Linux进程的管理与调度（七）](http://blog.csdn.net/gatieme/article/details/51566690)
- [Linux下进程的创建过程分析(_do_fork/do_fork详解)–Linux进程的管理与调度（八）](http://blog.csdn.net/gatieme/article/details/51569932)
- [http://blog.csdn.net/gatieme/article/details/51577479](http://blog.csdn.net/gatieme/article/details/51577479)
- [Linux内核线程kernel thread详解–Linux进程的管理与调度（十）](http://blog.csdn.net/gatieme/article/details/51589205)
## 3.进程的加载与运行 ##
- [Linux进程启动过程分析do_execve(可执行程序的加载和运行)—Linux进程的管理与调度（十一）](http://blog.csdn.net/gatieme/article/details/51594439)
- [LinuxELF文件格式详解–Linux进程的管理与调度（十二）](http://blog.csdn.net/gatieme/article/details/51615799)
- [ELF文件的加载过程(load_elf_binary函数详解)–Linux进程的管理与调度（十三）](http://blog.csdn.net/gatieme/article/details/51628257)
## 4.进程的退出 ##
- [Linux进程退出详解(do_exit)–Linux进程的管理与调度(十四)）](http://blog.csdn.net/gatieme/article/details/51638706)
## 5.进程的调度 ##
- [Linux进程调度器概述–Linux进程的管理与调度(十五）](http://blog.csdn.net/gatieme/article/details/51699889)
- [Linux进程调度策略的发展和演变–Linux进程的管理与调度(十六）](http://blog.csdn.net/gatieme/article/details/51701149)
- [Linux进程调度器的设计–Linux进程的管理与调度(十七）](http://blog.csdn.net/gatieme/article/details/51702662)
- [Linux核心调度器之周期性调度器scheduler_tick–Linux进程的管理与调度(十八）](http://blog.csdn.net/gatieme/article/details/51872561)
- [Linux进程核心调度器之主调度器–Linux进程的管理与调度(十九）](http://blog.csdn.net/gatieme/article/details/51872594)
- [Linux用户抢占和内核抢占详解(概念, 实现和触发时机)–Linux进程的管理与调度(二十）](http://blog.csdn.net/gatieme/article/details/51872618)
- [Linux进程上下文切换过程context_switch详解–Linux进程的管理与调度(二十一）](http://blog.csdn.net/gatieme/article/details/51872659)
- [Linux进程优先级的处理–Linux进程的管理与调度(二十二)](http://blog.csdn.net/gatieme/article/details/51719208)
- [Linux唤醒抢占—-Linux进程的管理与调度(二十三）](http://blog.csdn.net/gatieme/article/details/51872831)
## 6.调度普通进程-完全公平调度器CFS ##
- [Linux进程调度之CFS调度器概述–Linux进程的管理与调度(二十四）](http://blog.csdn.net/gatieme/article/details/52067518)
- [Linux CFS调度器之负荷权重load_weight–Linux进程的管理与调度(二十五）](http://blog.csdn.net/gatieme/article/details/52067665)
- [Linux CFS调度器之虚拟时钟vruntime与调度延迟–Linux进程的管理与调度(二十六）](http://blog.csdn.net/gatieme/article/details/52067748)
- [Linux CFS调度器之队列操作–Linux进程的管理与调度(二十七）](http://blog.csdn.net/gatieme/article/details/52067898)
- [Linux CFS调度器之pick_next_task_fair选择下一个被调度的进程–Linux进程的管理与调度(二十八）](http://blog.csdn.net/gatieme/article/details/52068016)
- [Linux CFS调度器之task_tick_fair处理周期性调度器–Linux进程的管理与调度(二十九）](http://blog.csdn.net/gatieme/article/details/52068050)
- [Linux CFS调度器之唤醒抢占–Linux进程的管理与调度(三十）](http://blog.csdn.net/gatieme/article/details/52068061)
***
#### 原文链接 ####
- [Linux进程管理与调度](https://blog.csdn.net/gatieme/article/details/51456569)