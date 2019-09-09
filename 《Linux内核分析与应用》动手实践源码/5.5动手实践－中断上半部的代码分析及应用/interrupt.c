# include <linux/kernel.h>
# include <linux/init.h>
# include <linux/module.h>
# include <linux/interrupt.h>

static int irq;					
static char * devname;			
			

module_param(irq,int,0644);
module_param(devname,charp,0644);
			
struct myirq
{
    int devid;
};

struct myirq mydev={1119};
		
//中断处理函数
static irqreturn_t myirq_handler(int irq,void * dev)
{
    struct myirq mydev;
    static int count=1;
    mydev = *(struct myirq*)dev;		
    printk("key: %d..\n",count);
    printk("devid:%d ISR is working..\n",mydev.devid);
    printk("ISR is leaving......\n");
    count++;
    return IRQ_HANDLED;
}


//内核模块初始化函数
static int __init myirq_init(void)
{
    printk("Module is working...\n");
    if(request_irq(irq,myirq_handler,IRQF_SHARED,devname,&mydev)!=0)
    {
        printk("%s request IRQ:%d failed..\n",devname,irq);
        return -1;
    }
    printk("%s request IRQ:%d success...\n",devname,irq);
    return 0;
}

//内核模块退出函数
static void __exit myirq_exit(void)
{
    printk("Module is leaving...\n");
    free_irq(irq,&mydev);
    printk("Free the irq:%d..\n",irq);
}

MODULE_LICENSE("GPL");
module_init(myirq_init);
module_exit(myirq_exit);