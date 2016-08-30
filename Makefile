obj-m  := Main.o
KDIR := /usr/src/d2plug-linux-2.6.32.y
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-

default :
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean :
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
