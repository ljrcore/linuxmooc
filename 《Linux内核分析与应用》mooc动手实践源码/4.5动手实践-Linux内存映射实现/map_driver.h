#include <asm/atomic.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>

struct mapdrvo
{
	struct cdev mapdev;
	atomic_t usage;
};

