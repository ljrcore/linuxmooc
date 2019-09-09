OS_VER := UNKNOWN
UNAME := $(shell uname -r)
ifneq ($(findstring 4.15.0-39-generic,$(UNAME)),)
		OS_VER := UBUNTU_1604
endif

ifneq ($(KERNELRELEASE),)
		obj-m += $(MODNAME).o
		$(MODNAME)-y := main.o
		ccflags-y := -I$(PWD)/
else
		export PWD=`pwd`

ifeq ($(KERNEL_BUILD_PATH),)
		KERNEL_BUILD_PATH := /lib/modules/`uname -r`/build
endif
ifeq ($(MODNAME),)
		export MODNAME=load_monitor
endif

all:
		make CFLAGS_MODULE=-D$(OS_VER) -C /lib/modules/`uname -r`/build M=`pwd`  modules
clean:
		make -C $(KERNEL_BUILD_PATH) M=$(PWD) clean
endif


