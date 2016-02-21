obj-y := init.o kprintf.o

obj-$(CONFIG_SYS_SMP) += lock.o
obj-$(CONFIG_KERNEL_STAT) += stat.o
obj-$(CONFIG_KERNEL_TEST) += test.o


# flags
cppflags-$(CONFIG_AVR) := -Os
