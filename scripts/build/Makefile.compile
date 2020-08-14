#
# Copyright (C) 2015 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



##############
### helper ###
##############

# indicate if a command file has to be generated for <target> and/or <cmd>
#
# 	$(call cmd_file_required,<cmd>,<target>)
define cmd_file_required
$(findstring compile,$(strip $(1)))
endef

# update a command file ($@.cmd)
# 	if <action> is "check" the command file is only touched if an update is required
# 	otherwise <cmd> is written to the command file
#
# 	NOTE commands that contain '$' will deviate from the actual command such that
# 		 '$' are missing
#
#	$(call update_cmd_file,<action>,<cmd>)
define update_cmd_file
	$(eval cmd=$(strip $(shell echo '$(2)' | sed -e "s/^@\(.*\)/\1/"))) \
	$(if $(findstring check,$(1)),
		$(eval old=$(shell cat $@.cmd 2>/dev/null)) \
		$(eval added=$(filter-out $(old),$(cmd))) \
		$(eval removed=$(filter-out $(cmd),$(old))) \
		$(QUTIL) [ '$(cmd)' = "$$(cat $@.cmd 2>/dev/null)" ] || { echo [TOUCH] $@.cmd $(if $(WHATCHANGED),\(+: $(added), -: $(removed), u: $?\)); echo '$(cmd)' > $@.cmd; } \
		, \
		$(QBUILD)echo '$(cmd)' > $@.cmd \
	)
endef

# generate a dependency file for the current target
# 	dependency file is not generated while executing prestage stage0
# 	fixdep is only applied if configtools are configured to be used
# 	and built already
#
#	$(call gen_dep_file,<compiler>,<compile-flags>)
define gen_dep_file
	$(if $(call is_prestage,stage0), \
		, \
		$($(1)) $(filter-out %.cmd,$(2)) -MM -MF $@.d -MP -MT $@ $<
		$(if $(configtools_unavailable), \
			,
			$(mv) $@.d $@.d.tmp
			$(fixdep) $@.d.tmp $(config_header) $(dir $(config_header))fixdep/ 1> $@.d
		) \
	)
endef

# compile function wrapping some use output and the command file generation
#
#	$(call compile_base,<compiler>,<args>)
define compile_base
	$(if $(call is_prestage,stage0),
		$(call update_cmd_file,check,$($(1)) $(2))
		,
		$(call update_cmd_file,build,$($(1)) $(2))
		$(echo) [$(call upper_case,$(1))] $@ $(if $(WHATCHANGED),\($?\))
		$($(1)) $(filter-out %.cmd,$(2))
	)
endef

# compile function extending $(compile_base) by dependency file generation
#
#	$(call compile_with_deps,<compiler>,<compile-flags>,<mode-flags>)
define compile_with_deps
	$(call compile_base,$(1),$(2) $(3) $< -o $@)
	$(call gen_dep_file,$(1),$(2))
endef


##########################
###   build commands   ###
##########################

# XXX naming: compile_<target type>_<dependencies type>
# XXX $(call compile_*,<host>)

define compile_c_y
	$(call compile_base,yacc,$(yaccflags) -v --report-file=$(basename $@).log --defines=$(basename $@).h $< -o $@)
endef

define compile_c_l
	$(call compile_base,lex,$(lexflags) --header-file=$(basename $@).h -o $@ $<)
endef

define compile_c_gperf
	$(call compile_base,gperf,$(gperfflags) $< --output-file=$@)
	$(gperf_c_header) $< $@ $(basename $@).h
endef

define compile_cxx_gperf
	$(call compile_base,gperf,$(gperfflags) $< --output-file=$@)
	$(gperf_cxx_header) $@ $(basename $@).h
endef

# define 'ASM' for preprocessed assembly files
%.S.i: cppflags += -DASM

define compile_i_c
	$(call compile_with_deps,$(1)cc,$($(1)cppflags) $($(1)archflags),-E)
endef

define compile_i_cxx
	$(call compile_with_deps,$(1)cxx,$($(1)cppflags) $($(1)archflags),-E)
endef

define compile_s_c
	$(call compile_with_deps,$(1)cc,$($(1)cflags) $($(1)cppflags) $($(1)archflags),-S)
endef

define compile_s_cxx
	$(call compile_with_deps,$(1)cxx,$($(1)cxxflags) $($(1)cppflags) $($(1)archflags),-S)
endef

define compile_o_s
	$(call compile_base,$(1)as,$($(1)asflags) $($(1)archflags) $< -o $@)
endef

define compile_o_c
	$(call compile_with_deps,$(1)cc,$($(1)cflags) $($(1)cppflags) $($(1)archflags),-c)
endef

define compile_o_cxx
	$(call compile_with_deps,$(1)cxx,$($(1)cxxflags) $($(1)cppflags) $($(1)archflags),-c)
endef

ifeq ($(project_type),c)
define compile_o_o
	$(eval flags := $($(1)cflags) -nostdlib -r -Wl,-r$(if $(strip $($(1)ldflags)),$(comma))$(subst $(space),$(comma),$(strip $($(1)ldflags))))
	$(call compile_base,$(1)cc,$(flags) $(filter %.o,$^) -o $@)
endef

else

define compile_o_o
	$(eval flags := $($(1)cxxflags) -nostdlib -r -Wl,-r$(if $(strip $($(1)ldflags)),$(comma))$(subst $(space),$(comma),$(strip $($(1)ldflags))))
	$(call compile_base,$(1)cxx,$(flags) $(filter %.o,$^) -o $@)
endef

endif

define compile_lib_o
	$(call compile_base,$(1)ar,crs $@ $(filter %.o,$^))
endef

ifeq ($(project_type),c)
define compile_bin_o
	$(call compile_base,$(1)cc,$($(1)archflags) $(filter %.o,$^) -o $@ $($(1)ldlibs))
endef
else
define compile_bin_o
	$(compile_base,($(1)cxx $($(1)archflags) $(filter %.o,$^) -o $@ $($(1)ldlibs))
endef
endif

# execute <script> during all stages except stage0
#
#	$(call cmd_run_script,<script>
define cmd_run_script
	$(call skip_prestage,stage0,
		$(QBUILD)$(1)
	)
endef

# execute <script> during all stages except stage0
#
#	$(call compile_file,<script>
define compile_file
	$(call cmd_run_script,
		$(mkdir) $(dir $@)
		$(QUTIL)$(1)
	)
endef

define cmd_defconfig
	$(echo) [CP] $< '->' $(config)
	@(test -e $(config) && cp $(config) $(config).old) ; exit 0
	$(cp) $< $(config)
endef
