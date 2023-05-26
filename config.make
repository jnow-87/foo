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

.PHONY: allconfigs
allconfigs:
	$(call cmd_run_script, \
		configs=$$(ls -1 $(config_tree)); \
		\
		for cfg in $${configs}; do \
			cp $(config_tree)/$${cfg} $(config); \
			make menuconfig; \
			cp $(config) $(config_tree)/$${cfg}; \
		done \
	)

.PHONY: allbuilds
allbuilds:
	$(call cmd_run_script, \
		configs=$$(ls -1 $(config_tree)); \
		build_log=$$(echo $(default_build_tree)/allbuilds.log | tr -s '/'); \
		\
		rm -f $${build_log}; \
		echo "build log:" $(call fg,violet,"$${build_log}"); \
		\
		for cfg in $${configs}; do \
			echo "building config $${cfg}"; \
			cp $(config_tree)/$${cfg} $(config); \
			make --no-print-directory >> $${build_log} 2>&1 || exit 1; \
		done \
	)
endif

# default config targets
ifneq ("$(config_tree)","")
config_files := $(notdir $(wildcard $(config_tree)/*))

$(foreach cfg, $(config_files), \
	$(call gen_rule_basic,cmd_defconfig,defconfig-$(cfg),$(config_tree)/$(cfg)) \
)
endif
