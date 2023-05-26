#
# Copyright (C) 2014 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



####
## include helper
####

include $(scripts_dir)/utils.make
include $(scripts_dir)/escape.make


####
## init
####

# include config file if defined
ifneq ("$(config)","")
  # warn if $(config) does not exist and more than the following targets are called
  ifneq ($(filter-out menuconfig defconfig% clean% %clean,$(MAKECMDGOALS)),)
    ifeq ("$(wildcard $(config))","")
      $(warning $(config) does not exist, run $$make menuconfig or $$make defconfig-* first)
    endif
  endif

  -include $(config)
endif

include $(scripts_dir)/init.make
include $(scripts_dir)/debug.make

# execute prestages
include $(scripts_dir)/stages.make


####
## include compile and rule generation helper
####

include $(scripts_dir)/compile.make
include $(scripts_dir)/compile_cc.make
include $(scripts_dir)/genrule.make


####
## basic targets
####

# the default target
.PHONY: all
all: check_user_tools check_build_tools check_config dotclang

# target used to force recipe execution of dependent targets
.PHONY: force
force:

# fake target for non-existent command files
%.cmd:
	@:

# target used by prestage stage1, intented for targets that
# generate files that are included by the build system and
# hence have to be build in a separate build run
# it might also be used for targets that are not listed as
# dependencies to others but are still required
prepare_deps:
	@:


####
## subsequent targets
####

ifeq ("$(use_config_sys)","y")
  include $(scripts_dir)/config.make
else
  configtools_unavailable := y
endif

include $(scripts_dir)/check.make

include $(scripts_dir)/versionheader.make
include $(scripts_dir)/clang.make

ifneq ("$(githooks_tree)","")
  include $(scripts_dir)/githooks.make
endif

include $(scripts_dir)/test.make

ifeq ("$(use_coverage_sys)","y")
  include $(scripts_dir)/coverage.make
 endif

include $(scripts_dir)/help.make

# include dependency files
include $(shell find $(build_tree)/ -type f -name \*.d 2>/dev/null)


####
## traverse project makefiles
####

include $(scripts_dir)/traverse.make

# start subdirectory traversal
# 	if subdir-y is empty include '.' (within $(src_tree))
$(if $(src_dirs), \
	$(call dinclude,$(src_dirs) $(mconfig_src) $(fixdep_src) $(confheader_src)) \
	, \
	$(error $$(src_dirs) is empty, please define initial source directories) \
)
