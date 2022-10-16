#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-nobuiltin-y := libbrick.o

# check if $(arch) is empty to avoid make infinite loop
ifneq ($(CONFIG_ARCH),)
  obj-y += $(CONFIG_ARCH)/
  libbrick-y += $(loc_build_tree)/$(CONFIG_ARCH)/libbrick.o
endif
