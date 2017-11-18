obj-y := init.o thread.o process.o sched.o panic.o kmem.o umem.o page.o binloader.o binloader_raw.o syscall.o lock.o fs/

obj-$(CONFIG_KERNEL_STAT) += stat.o
obj-$(CONFIG_KERNEL_TEST) += test.o


config_kprintf := $(findstring y,$(CONFIG_KERNEL_MSG_FATAL) $(CONFIG_KERNEL_MSG_WARN) $(CONFIG_KERNEL_MSG_DEBUG) $(CONFIG_KERNEL_MSG_INFO) $(CONFIG_KERNEL_STAT))
obj-$(config_kprintf) += kprintf.o


####
# flags
####

## avr
# disable jump tables if kernel is mapped to addresses larger than 64k
# this is required due to avr-gcc implementation of jump tables
ifeq ($(shell test $$(printf "%u" $(CONFIG_KERNEL_TEXT_BASE)) -gt $$(printf "%u" 0x10000); echo $$?),0)
  cflags-$(CONFIG_AVR) := -fno-jump-tables
endif

## debug messages
syscall-cppflags := -DBUILD_KERNEL_SYSCALL
lock-cppflags := -DBUILD_KERNEL_SMP
