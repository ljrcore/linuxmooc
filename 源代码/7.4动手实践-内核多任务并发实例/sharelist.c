#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/spinlock_types.h>
#include <linux/workqueue.h>
#include <linux/slab.h>        /*kmalloc的头文件*/
#include <linux/kthread.h>
#include <linux/kallsyms.h>

#define NTHREADS 200 /* 线程数 */

struct my_struct {
	struct list_head list;
	int id;
	int pid;
};
static struct work_struct queue;
static struct timer_list mytimer;   /* 用于定时器队列 */
static LIST_HEAD(mine);  /* sharelist头 */
static unsigned int list_len = 0; 
static DEFINE_SEMAPHORE(sem);  /* 内核线程启动器之间进行同步的信号量,4.15内核适用*/
static DEFINE_SPINLOCK(my_lock); /* 保护对链表的操作,4.15内核适用 */
static atomic_t my_count = ATOMIC_INIT(0); /* 以原子方式进行追加 */
static int count = 0;

static int sharelist(void *data);
static void start_kthread(void);
static void kthread_launcher(struct work_struct *q);

/* 内核线程，把节点加到链表 */
static int sharelist(void *data)
{
        
     	struct my_struct *p;

	if (count++ % 4 == 0)
		printk("\n");

	spin_lock(&my_lock); /* 添加锁，保护共享资源 */
	if (list_len < 50) {
		if ((p = kmalloc(sizeof(struct my_struct), GFP_KERNEL)) == NULL)
			return -ENOMEM;
		p->id = atomic_read(&my_count); /* 原子变量操作 */
		atomic_inc(&my_count);
		p->pid = current->pid;
		list_add(&p->list, &mine); /* 向队列中添加新字节 */
		list_len++;
		printk("THREAD ADD:%-5d\t", p->id);
	}
      else { /* 队列超过定长则删除节点 */
		struct my_struct *my = NULL;
		my = list_entry(mine.prev, struct my_struct, list);
		list_del(mine.prev); /* 从队列尾部删除节点 */
		list_len--;
		printk("THREAD DEL:%-5d\t", my->id);
		kfree(my);
	}
	spin_unlock(&my_lock);
	return 0;
}

/* 调用keventd来运行内核线程 */
static void start_kthread(void)
{
	down(&sem);
	schedule_work(&queue);
}

static void kthread_launcher(struct work_struct *q)
{ 
          kthread_run(sharelist, NULL, "%d", count);
          up(&sem);
}

void qt_task(struct timer_list *timer)
{
        spin_lock(&my_lock);
	if (!list_empty(&mine)) {
		struct my_struct *i;
		if (count++ % 4 == 0)
			printk("\n");
		i = list_entry(mine.next, struct my_struct, list); /* 取下一个节点 */
		list_del(mine.next); /* 删除节点 */
		list_len--;
		printk("TIMER DEL:%-5d\t", i->id);
		kfree(i);
	}
        spin_unlock(&my_lock);
	mod_timer(timer, jiffies + msecs_to_jiffies(1000));
}


static int share_init(void)
{
    
        int i;
	printk(KERN_INFO"share list enter\n");
 
	INIT_WORK(&queue, kthread_launcher);
	timer_setup(&mytimer, qt_task, 0);
	add_timer(&mytimer);
	for (i = 0; i < NTHREADS; i++) 
		start_kthread();
	return 0;
}
static void share_exit(void)
{
	struct list_head *n, *p = NULL;
	struct my_struct *my = NULL;
	printk("\nshare list exit\n");
	del_timer(&mytimer);
	spin_lock(&my_lock); /* 上锁，以保护临界区 */
	list_for_each_safe(p, n, &mine)
        { /* 删除所有节点，销毁链表 */
		if (count++ % 4 == 0)
			printk("\n");
		my = list_entry(p, struct my_struct, list); /* 取下一个节点 */
		list_del(p);
		printk("SYSCALL DEL: %d\t", my->id);
		kfree(my);

	}
	spin_unlock(&my_lock); /* 开锁 */	
	printk(KERN_INFO"Over \n");
}

module_init(share_init);
module_exit(share_exit);

MODULE_LICENSE("GPL v2");

