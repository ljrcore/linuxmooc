# include <linux/kernel.h>
# include <linux/module.h>
# include <uapi/linux/sched.h>
# include <linux/init_task.h>
# include <linux/init.h>
# include <linux/fdtable.h>
# include <linux/fs_struct.h>
# include <linux/mm_types.h>

MODULE_LICENSE("GPL");

//内核模块初始化函数
static int __init print_pid(void)
{
	struct task_struct *task, *p;
	struct list_head *pos;
	int count=0;
	printk("Printf process'message begin:\n");
	task = &init_task;
	
	//遍历进程链表
	list_for_each(pos,&task->tasks)
	{
		p = list_entry(pos,struct task_struct,tasks);
		count++;
		printk("\n\n");
		printk("pid:%d; state:%lx; prio:%d; static_prio:%d; parent'pid:%d; count:%d; umask:%d;",	\
			p->pid,p->state,p->prio,p->static_prio,(p->parent)->pid,								\
			atomic_read((&(p->files)->count)),(p->fs)->umask);
		
		if((p->mm)!=NULL)
			printk("total_vm:%ld;",(p->mm)->total_vm);
	}
	
	printk("进程的个数：%d\n",count);	

	return 0;
}

//内核模块退出函数
static void __exit pid_exit(void)
{
	printk("exiting...\n");
}


module_init(print_pid);
module_exit(pid_exit);