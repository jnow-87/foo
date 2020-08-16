#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



####
## check build tools
####

check_build_tools_error := "can't find \"$${val}\", ensure the variables $${id} or CONFIG_$${id} to be initialised correctly"

build_tools :=
$(foreach tool,$(all_build_tools), \
	$(eval build_tools += $(call upper_case,$(tool))=$(subst @,,$($(tool)))) \
)

.PHONY: check_build_tools
check_build_tools:
	$(call cmd_run_script, \
		@r=0; \
		for tool in $(build_tools); do \
			id=$$(echo $${tool} | cut -d '=' -f 1); \
			val=$$(echo $${tool} | cut -d '=' -f 2); \
			test -n "$$(which $${val})" || (echo $(check_build_tools_error); exit 1); \
			test $${?} -eq 1 && r=1; \
		done; \
		exit $${r}; \
	)


####
## check user tools
####

check_user_tools_error := "can't find \"$${tool}\", ensure it is installed"

.PHONY: check_user_tools
check_user_tools:
	$(call cmd_run_script, \
		@r=0; \
		for tool in $(all_user_tools); do \
			test -n "$$(which $${tool})" || (echo $(check_user_tools_error); exit 1); \
			test $${?} -eq 1 && r=1; \
		done; \
		exit $${r} \
	)

####
## check $(config)
####

.PHONY: check_config
check_config:
ifneq ($(shell test -e $(config) && echo $(config)),$(config))
	$(call error,$(config) does not exist, please run $$make menuconfig or $$make defconfig-<target> first)
endif
