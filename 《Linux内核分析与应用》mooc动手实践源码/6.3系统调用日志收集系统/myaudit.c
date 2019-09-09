#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <asm/current.h>

void (*my_audit) (int,int) = 0;

int(*my_sysaudit)(u8,u8 *,u16,u8) = 0;
SYSCALL_DEFINE4(myaudit,u8,type,u8 *,us_buf,u16,us_buf_size,u8,reset)
{
        if (my_audit){
                printk("IN KENEL:my system call sys_myaudit() working\n");
                return (*my_sysaudit)(type,us_buf,us_buf_size,reset);

        } else
                printk("my_audit is not exist\n");
        return 1;

}

EXPORT_SYMBOL(my_audit);
EXPORT_SYMBOL(my_sysaudit);

