KDIR = /lib/modules/$(shell uname -r)/build
PWD  = $(shell pwd)
MAKE = /usr/bin/make
obj-m += opgave_4_1.o
device_name = opgave_4_1
major_number = 243

%.ko : %.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod $(device_name).ko

uninstall:
	sudo rmmod $(device_name)
