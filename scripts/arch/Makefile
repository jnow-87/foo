#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# check if CONFIG_ARCH is set and a corresponding directory exists
# to avoid overwriting make targets
ifneq ($(CONFIG_ARCH),)
  ifneq ($(wildcard $(loc_src_tree)/$(CONFIG_ARCH)),)
    hostobj-y += $(CONFIG_ARCH)/
  endif
endif
