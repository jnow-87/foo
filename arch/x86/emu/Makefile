#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := x86emu


x86emu-y := main.o linux.o hardware/ brickos/ user/

# ensure a rebuild if indirect dependencies are updated
x86emu-y += $(build_tree)/sys/libhost.a


x86emu-hostcppflags += \
	-I$(src_tree)/include \
	-I$(loc_src_tree)

x86emu-hostcflags += -fno-builtin

x86emu-hostldlibs += \
	-L$(build_tree)/sys/ -lhost	\
	-lpthread \
	-lreadline \
	-lrt \
	-lX11
