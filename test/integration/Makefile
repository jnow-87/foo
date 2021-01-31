#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



subdir-y := tests/
hostbin-y := itest

itest-y := main.o hardware/ brickos/ user/


itest-hostcppflags += \
	-I$(loc_src_tree) \
	-I$(src_tree)

itest-hostcflags += \
	-fno-builtin

itest-hostldlibs += \
	-lpthread \
	-lreadline \
	-lrt


test: $(loc_src_tree)/itest.sh
coverage: $(loc_build_tree)/itest
