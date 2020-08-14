#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := main


main-y := main.o string.o vfprintf.o list.o list1.o queue.o stack.o memblock.o vector.o sys/

main-$(CONFIG_SYS_RINGBUFFER) += ringbuf.o
main-$(CONFIG_SYS_PATTERN_MATCH) += patmat.o

main-hostcppflags += -I$(src_tree) -I$(src_tree)/include -I/usr/include
main-hostcflags += -fno-builtin


# testing rules
.PHONY: test
test: $(loc_build_tree)/main
	$(call cmd_run_script,$< $(build_tree)/testing/result.log)
