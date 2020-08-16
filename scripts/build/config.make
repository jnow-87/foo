#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



ifneq ("$(config_ftype)","")
  config_header := $(build_tree)/config/config.h

  mconfig_src := $(scripts_dir)/mconf
  mconfig := $(build_tree)/$(mconfig_src)/mconfig
  confheader_src := $(scripts_dir)/mconf
  confheader := $(build_tree)/$(confheader_src)/confheader
  fixdep_src := $(scripts_dir)/fixdep
  fixdep := $(build_tree)/$(fixdep_src)/fixdep

  config_tools := $(fixdep) $(mconfig) $(confheader)

  fixdep := $(QBUILD)$(fixdep)

configtools: configtools_unavailable := y
configtools: $(config_tools)

$(config_header): $(config) configtools
	$(call compile_file,KCONFIG_CONFIG=$(config) $(confheader) $(config_ftype) $(dir $(config_header))fixdep $(config_header))

all: $(config_header)
prepare_deps: $(config_header)

.PHONY: menuconfig
menuconfig: check_build_tools configtools
	$(call cmd_run_script,KCONFIG_CONFIG=$(config) $(mconfig) $(config_ftype))

endif

# default config targets
ifneq ("$(config_tree)","")
config_files := $(notdir $(wildcard $(config_tree)/*))

$(foreach cfg, $(config_files), \
	$(call gen_rule_basic,cmd_defconfig,defconfig-$(cfg),$(config_tree)/$(cfg)) \
)
endif
