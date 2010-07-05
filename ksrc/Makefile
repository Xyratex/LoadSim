ifneq ($(KERNELRELEASE),)
    obj-m	:= lve.o
    lve-objs	:= light_ve.o lve-mod.o tags.o lve_procfs.o lve_list.o
else
    KDIR        := /lib/modules/2.6.18-128.2.1.el5.028stab064.8-lve/build
    PWD         := $(shell pwd)

debug:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) EXTRA_CFLAGS="-DLVE_DEBUG=1 -g" modules 

default: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
endif

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f Module.markers Module.symvers


