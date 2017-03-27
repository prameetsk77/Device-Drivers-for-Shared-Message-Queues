#KDIR:= ~/ESP/SDK/sysroots/i586-poky-linux/usr/src/kernel/
KDIR:=/lib/modules/$(shell uname -r)/build
PWD:= $(shell pwd)

# CC = i586-poky-linux-gcc
# ARCH = x86
# CROSS_COMPILE = i586-poky-linux-
# SROOT= /home/esp/SDK/sysroots/i586-poky-linux/

APP1 = main

obj-m:= Squeue.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc Main.c -lpthread  -o $(APP1) 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f $(APP1)
	
