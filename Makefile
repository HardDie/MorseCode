obj-m  := Main.o
KDIR := /usr/src/linux-headers-4.2.0-27-generic

default :
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean :
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

dove : KDIR := /usr/src/d2plug-linux-2.6.32.y
dove : export ARCH=arm
dove : export CROSS_COMPILE=arm-linux-gnueabi-
dove : default
