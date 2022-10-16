#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := syscall.o file.o signal.o process.o time.o shm.o mmap.o

# libbrick
obj-nobuiltin-y := libbrick.o

libbrick-y := $(patsubst %.o,%.libbrick.o,$(obj-y))
