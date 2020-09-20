#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := gen_lds_memory

gen_lds_memory-y := lds_memory.o sys/


hostcppflags-y := -I"include/"
hostcflags-y := \
	-Wno-unused-function \
	-fno-builtin

hostldlibs-y += -fprofile-arcs
