# Si KERNELRELEASE est defini on peut compiler !

ifneq ($(KERNELRELEASE),)
    obj-m := messag.o
else
   KERNELDIR ?= /lib/modules/$(shell uname -r)/build
   PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cc -o test tmmap.c
	cc -o test_protocol test_protocol.c
clean :
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f test test_protocol
endif
