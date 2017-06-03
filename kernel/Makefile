obj-y := init.o sched.o panic.o kmem.o umem.o

obj-$(CONFIG_KERNEL_SMP) += lock.o
obj-$(CONFIG_KERNEL_STAT) += stat.o
obj-$(CONFIG_KERNEL_TEST) += test.o


config_kprintf := $(findstring y,$(CONFIG_KERNEL_MSG_FATAL) $(CONFIG_KERNEL_MSG_WARN) $(CONFIG_KERNEL_MSG_DEBUG) $(CONFIG_KERNEL_MSG_INFO) $(CONFIG_KERNEL_MSG_STAT))
obj-$(config_kprintf) += kprintf.o


# flags
cppflags-$(CONFIG_AVR) := -Os
