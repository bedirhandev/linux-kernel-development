kdir := /home/bedirhan/linux # locatie kernel
cdir = $(shell pwd)
MAKE = /usr/bin/make
obj- += gpio_extern.o
device_name = gpio_extern

# en bouwomgeving. Mag ook ${PWD}
# zijn, de naam van de huidige directory
%.ko : %.c
	$(MAKE) $(*).ko obj-m=$(*).o -C ${kdir} M=${cdir} ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules

clean:
	$(MAKE) -C $(kdir) M=$(cdir) clean

install:
	sudo insmod $(device_name).ko

uninstall:
	sudo rmmod $(device_name)
