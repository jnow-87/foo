#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := syscall.o file.o signal.o process.o time.o

# libsys
obj-nobuiltin-y := libsys.o

libsys-y := $(patsubst %.o,%.libsys.o,$(obj-y))
