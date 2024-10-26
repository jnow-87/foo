#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := dtinfo

dtinfo-y := main.o sys/


hostcppflags-y := -Iinclude
hostcflags-y := -fno-builtin


####
## targets
####

dtinfo := $(loc_build_tree)/dtinfo

devtree-info: $(dtinfo)
	$(call cmd_run_script,$<)
