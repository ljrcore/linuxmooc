# include <linux/module.h>
# include <linux/fs.h>
# include <linux/uaccess.h>
# include <linux/init.h>
# include <linux/cdev.h>
//加入misc机制
# include <linux/miscdevice.h>
# include <linux/kfifo.h>

DEFINE_KFIFO(mydemo_fifo,char,64);

//设备名
# define DEMO_NAME "my_demo_dev"

static struct device *mydemodrv_device;

static int demodrv_open(struct inode *inode, struct file *file)
{
	int major = MAJOR(inode->i_rdev);
	int minor = MINOR(inode->i_rdev);

	printk("%s: major=%d, minor=%d\n",__func__,major,minor);

	return 0;
}


static ssize_t demodrv_read(struct file *file, char __user *buf,size_t count,loff_t *ppos)
{
	int actual_readed;
	int ret;

	ret = kfifo_to_user(&mydemo_fifo,buf, count, &actual_readed);
	if(ret)
		return -EIO;

	printk("%s,actual_readed=%d,pos=%lld\n",__func__,actual_readed,*ppos);

	return actual_readed;
}


static ssize_t demodrv_write(struct file *file, const char __user *buf,size_t count,loff_t *ppos)
{
	unsigned int actual_write;
	int ret;

	ret = kfifo_from_user(&mydemo_fifo,buf, count, &actual_write);
	if(ret)
		return -EIO;

	printk("%s: actual_write=%d,ppos=%lld\n",__func__,actual_write,*ppos);

	return actual_write;
}


static const struct file_operations demodrv_fops = {
	.owner = THIS_MODULE,
	.open = demodrv_open,
	.read = demodrv_read,
	.write = demodrv_write,
};

static struct miscdevice mydemodrv_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEMO_NAME,
	.fops = &demodrv_fops,
};

static int __init simple_char_init(void)
{
	int ret;

	ret = misc_register(&mydemodrv_misc_device);
	if(ret)
	{
		printk("failed register misc device\n");
		return ret;
	}

	mydemodrv_device = mydemodrv_misc_device.this_device;

	printk("successed register char device: %s\n",DEMO_NAME);

	return 0;
} 


static void __exit simple_char_exit(void)
{
	printk("removing device\n");

	misc_deregister(&mydemodrv_misc_device);
}

module_init(simple_char_init);
module_exit(simple_char_exit);

MODULE_LICENSE("GPL");