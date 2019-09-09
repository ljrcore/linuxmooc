obj-m :=sharelist.o
CURRENT_PATH := $(shell pwd)
LINUX_KERNEL := $(shell uname -r)
LINUX_KERNEL_PATH := /lib/modules/$(shell uname -r)/build

all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules 
clean:
	@rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions Module.symvers .Makefile.swp modules.order *.o.ur-detected *.o.ur-safe	
