obj-$(CONFIG_DRIVER_LOOPBACK) += loop/
obj-$(CONFIG_DRIVER_UART) += uart/


####
# flags
####

## avr
# disable jump tables if kernel is mapped to addresses larger than 64k
# this is required due to avr-gcc implementation of jump tables
ifeq ($(shell test $$(printf "%u" $(CONFIG_KERNEL_TEXT_BASE)) -gt $$(printf "%u" 0x10000); echo $$?),0)
  cflags-$(CONFIG_AVR) := -fno-jump-tables
endif


## feature
cppflags-y := -DBUILD_DRIVER
