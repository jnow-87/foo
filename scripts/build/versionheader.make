#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# version header
versionheader_create := $(QBUILD)$(scripts_dir)/versionheader.sh
versionheader_header := $(build_tree)/version.h

.PHONY: versionheader
versionheader:
	$(call cmd_run_script,$(versionheader_create) $(versionheader_header))

prepare_deps: versionheader
