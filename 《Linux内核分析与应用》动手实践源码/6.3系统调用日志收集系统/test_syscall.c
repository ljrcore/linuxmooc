#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define COMM_SIZE 16

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

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

#define AUDIT_BUF_SIZE (20*sizeof(struct syscall_buf))

int main(int argc,char *argv[])
{	
	u8 col_buf[AUDIT_BUF_SIZE];
	unsigned char reset = 1;
	int num = 0;
	int i,j;
	struct syscall_buf *p;
	while(1){
		num = syscall(335,0,col_buf,AUDIT_BUF_SIZE,reset);
		printf("num :%d\n",num);
		p = (struct syscall_buf *)col_buf;
		for(i=0;i<num;i++){
			printf("num[%d],serial:%d,\t syscall :%d,\t pid:%d,\t comm:%s,\t time:%s\n",i,p[i].serial,p[i].syscall,p[i].pid,p[i].comm,ctime(&p[i].ts_sec));
		}
	}
	return 1;
}
