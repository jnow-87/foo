#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# check if $(arch) is empty to avoid make infinite loop
ifneq ($(wildcard $(loc_src_tree)/$(CONFIG_ARCH)),)
  hostobj-y += $(CONFIG_ARCH)/
endif
