ifneq ($(KERNELRELEASE),)
	obj-m += map_driver.o
else
	PWD := $(shell pwd)
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
default:
	$(MAKE) -C $(KERNELDIR)  M=$(PWD) modules
clean:
	@rm -rf *.o *.mod.c *.mod.o *.ko *.order *.symvers .*.cmd .tmp_versions
endif
