#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := itestfw


itestfw-y := main.o linux.o hardware/ brickos/ user/

# ensure a rebuild if indirect dependencies are updated
itestfw-y += $(build_tree)/sys/libhost.a


itestfw-hostcppflags += \
	-I$(src_tree)/include \
	-I$(loc_src_tree)

itestfw-hostcflags += -fno-builtin

itestfw-hostldlibs += \
	-L$(build_tree)/sys/ -lhost	\
	-lpthread \
	-lreadline \
	-lrt \
	-lX11
