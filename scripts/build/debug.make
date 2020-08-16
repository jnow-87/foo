#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# print DEBUG message
#
#	$(call pdebug,<msg>)
ifeq ($(call cond_ge,$(V),3),1)
  define pdebug0
    $(info $1)
  endef
endif

ifeq ($(call cond_ge,$(V),4),1)
  define pdebug1
    $(info $1)
  endef
endif

ifeq ($(call cond_ge,$(V),5),1)
  define pdebug2
    $(info $1)
  endef
endif
