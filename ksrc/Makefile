ifneq ($(KERNELRELEASE),)
    obj-m	:= sim.o
    sim-objs	:= simul.o 
    sim-objs	+= md_env.o clients/generic_md.o
    sim-objs	+= fifo.o vm_main.o vm_core.o vm_call.o vm_sys.o
else
    KDIR        := /lib/modules/$(shell uname -r)/build/
    PWD         := $(shell pwd)
    ROOT	:= $(shell cd ..; pwd)

debug:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) EXTRA_CFLAGS="-g -I $(ROOT)/include/ -I $(PWD)" modules 

default: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
endif

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f Module.markers Module.symvers


