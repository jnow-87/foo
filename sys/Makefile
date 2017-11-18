obj-y := errno.o string.o stream.o memblock.o ringbuf.o
obj-nobuiltin-y := libsys.o
hostobj-y := errno.o string.o stream.o memblock.o ringbuf.o

obj-$(CONFIG_KERNEL_SMP) += mutex.o

libsys-y := errno.o string.o stream.o memblock.o ringbuf.o mutex_lib.o


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%_lib.o,    $(loc_src_tree)/%.c,)


####
# flags
####

## avr
# disable jump tables if kernel is mapped to addresses larger than 64k
# this is required due to avr-gcc implementation of jump tables
ifeq ($(shell test $$(printf "%u" $(CONFIG_KERNEL_TEXT_BASE)) -gt $$(printf "%u" 0x10000); echo $$?),0)
  cflags-$(CONFIG_AVR) := -fno-jump-tables
endif
