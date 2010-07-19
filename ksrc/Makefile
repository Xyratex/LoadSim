ifneq ($(KERNELRELEASE),)
    obj-m	:= sim.o
    sim-objs	:= fifo.o vm_main.o
else
    KDIR        := /lib/modules/$(shell uname -r)/build/
    PWD         := $(shell pwd)

debug:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) EXTRA_CFLAGS="-g" modules 

default: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
endif

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f Module.markers Module.symvers


