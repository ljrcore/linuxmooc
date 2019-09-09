#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

#define COMM_SIZE 16
#define AUDIT_BUF_SIZE 20

MODULE_LICENSE("GPL v2");

struct syscall_buf{
u32 serial;
u64 ts_sec;
u64 ts_micro;
u32 syscall;
u32 status;
pid_t pid;
uid_t uid;
u8 comm[COMM_SIZE];	
};

DECLARE_WAIT_QUEUE_HEAD(buffer_wait);

static struct  syscall_buf  audit_buf[AUDIT_BUF_SIZE];
static int current_pos = 0;
static u32 serial = 0;

void syscall_audit(int syscall,int return_status)
{	
	struct syscall_buf *ppb_temp;
	struct timespec64 nowtime;
	
	ktime_get_real_ts64(&nowtime);
	if(current_pos<AUDIT_BUF_SIZE){
		ppb_temp = &audit_buf[current_pos];
		ppb_temp->serial = serial++;
		ppb_temp->ts_sec = nowtime.tv_sec;
		ppb_temp->ts_micro = nowtime.tv_nsec;
		ppb_temp->syscall = syscall;
		ppb_temp->status = return_status;
		ppb_temp->pid = current->pid;
		ppb_temp->uid = current->tgid; 
		
		memcpy(ppb_temp->comm,current->comm,COMM_SIZE);
		if(++current_pos ==AUDIT_BUF_SIZE * 6/10)
			{
				printk("IN MODULE_AUDIT:yes,it near full\n");
				wake_up_interruptible(&buffer_wait);
			}
		}
	
}

int sys_audit(u8 type,u8 *us_buf,u16 us_buf_size,u8 reset)
{
	int ret = 0;
		if(!type){
			if(clear_user(us_buf,us_buf_size)){
				printk("Error:clear_user\n");
				return 0;
			}
			printk("IN MODULE_systemcall:starting...\n");
			ret = wait_event_interruptible(buffer_wait,current_pos >= AUDIT_BUF_SIZE *6/10);
			printk("IN MODULE_systemcall:over,current_pos is %d\n",current_pos);
			if(copy_to_user(us_buf,audit_buf,(current_pos)*sizeof(struct syscall_buf))){
				printk("Error:copy error\n");
				return 0;
			}
			ret = current_pos;
			current_pos = 0;

		}
		return ret;		
}

extern void(*my_audit)(int,int);
extern int (*my_sysaudit)(u8,u8*,u16,u8);
static int __init audit_init(void)
{
	my_sysaudit = sys_audit;
	my_audit = syscall_audit;
	printk("Exiting System Call Auditing\n");
	return;
}

module_init(audit_init);

static void __exit audit_exit(void)
{
	my_audit = NULL;
	my_sysaudit = NULL;
	printk("Exiting System Calling Auditing\n");
	return;
}

module_exit(audit_exit);





