ifneq ($(KERNELRELEASE),)
    obj-m	:= mdsim.o
    mdsim-objs	:= fifo.o vm_main.o vm_call.o vm_math.o mdsimul.o
else
    KDIR        := 
    PWD         := $(shell pwd)

debug:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) EXTRA_CFLAGS="-g" modules 

default: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
endif

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f Module.markers Module.symvers


