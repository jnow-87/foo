#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := utest

utest-y := main.o testcase.o string.o stream.o list.o list1.o queue.o stack.o memory.o memblock.o vector.o inet.o devtree.o ringbuf.o patmat.o sys/


utest-hostcppflags += \
	-I$(loc_src_tree) \
	-I$(src_tree)/include

utest-hostcflags += \
	-fno-builtin \
	-fprofile-arcs \
	-ftest-coverage \
	--coverage

utest-hostldlibs += \
	-fprofile-arcs


test: $(loc_build_tree)/utest
coverage: $(loc_build_tree)/utest
