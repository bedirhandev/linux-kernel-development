KDIR = /lib/modules/$(shell uname -r)/build
PWD  = $(shell pwd)
MAKE = /usr/bin/make
obj-m += opgave_3_7.o
device_name = opgave_3_7
major_number = 243

%.ko : %.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod $(device_name).ko
	sudo mknod /dev/$(device_name) c ${major_number} 0

uninstall:
	sudo rmmod $(device_name)
	sudo rm /dev/$(device_name)
