# include <linux/init.h>
# include <linux/kernel.h>
# include <linux/module.h>

//内核模块初始化函数
static int __init lkm_init(void)
{
	printk("Hello World\n");
	return 0;
}

//内核模块退出函数
static void __exit lkm_exit(void)
{
	printk("Goodbye\n");
}

module_init(lkm_init);
module_exit(lkm_exit);

MODULE_LICENSE("GPL");