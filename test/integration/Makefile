#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := itestfw

itestfw-y := main.o hardware/ brickos/ user/


itestfw-hostcppflags += \
	-I$(loc_src_tree) \
	-I$(src_tree)

itestfw-hostcflags += \
	-fno-builtin

itestfw-hostldlibs += \
	-lpthread \
	-lreadline \
	-lrt
