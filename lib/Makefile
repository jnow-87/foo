#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#


lib-y := libsys.a
obj-y := init.o unistd/ stdlib/ stdio/ 

libsys-y := $(build_tree)/sys/libsys.o $(build_tree)/arch/libsys.o $(loc_build_tree)/obj.o

obj-$(CONFIG_SC_TIME) += time.o
obj-$(CONFIG_SC_SCHED) += sched.o
obj-$(CONFIG_SC_SIGNAL) += signal.o
obj-$(CONFIG_SC_SOCKET) += socket.o


####
# flags
####

libsys-cppflags-y += -DBUILD_LIBSYS
