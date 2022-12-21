#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#


lib-y := libbrick.a
obj-y := init.o unistd/ stdlib/ stdio/ 

libbrick-y := $(build_tree)/sys/libbrick.o $(build_tree)/arch/libbrick.o $(loc_build_tree)/obj.o

obj-$(CONFIG_SC_TIME) += time.o timer.o
obj-$(CONFIG_SC_SCHED) += sched.o
obj-$(CONFIG_SC_SIGNAL) += signal.o
obj-$(CONFIG_SC_SOCKET) += socket.o
obj-$(CONFIG_LIB_GETOPT) += getopt.o
obj-$(CONFIG_LIB_GFX) += gfx.o
obj-$(CONFIG_LIB_READLINE) += readline/

hostobj-$(CONFIG_LIB_GETOPT) += getopt.o

####
# flags
####

libbrick-cppflags-y += -DBUILD_LIBBRICK
