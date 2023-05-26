#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



####
## helper
####

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
	$(call gen_deps,$(1),$(2))
endef


####
## compile commands
####

# XXX naming: compile_<target type>_<dependencies type>
# XXX $(call compile_*,<host>)


define nop
endef

define compile_c_y
	$(call compile_base,yacc,$(yaccflags) -v --report-file=$(basename $@).log --defines=$(basename $@).h $< -o $@)
endef

define compile_c_l
	$(call compile_base,lex,$(lexflags) --header-file=$(basename $@).h -o $@ $<)
endef

define compile_c_gperf
	$(call compile_base,gperf,$(gperfflags) $< --output-file=$@)
	$(call cmd_run_script,$(gperf_c_header) $< $@ $(basename $@).h)
endef

define compile_cxx_gperf
	$(call compile_base,gperf,$(gperfflags) $< --output-file=$@)
	$(call cmd_run_script,$(gperf_cxx_header) $@ $(basename $@).h)
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
  o_o_cflags := cflags
  o_o_compiler := cc
else
  o_o_cflags := cxxflags
  o_o_compiler := cxx
endif

# NOTE
# 	to combine object files the compiler is used in favor of the linker since it is
# 	not easily possible to invoke the linker with the correct flags when link time
# 	optimisation (-flto) shall be used
define compile_o_o
	$(eval flags := -nostdlib -r $(filter-out %coverage -fprofile-arcs,$($(1)$(o_o_cflags))) -Wl,-r$(if $(strip $($(1)ldflags)),$(comma))$(subst $(space),$(comma),$(strip $($(1)ldflags))))
	$(call compile_base,$(1)$(o_o_compiler),$(flags) $(filter %.o,$^) -o $@)
endef

define compile_lib_o
	$(call compile_base,$(1)ar,crs $@ $(filter %.o,$^))
endef

ifeq ($(project_type),c)
define compile_bin_o
	$(call compile_base,$(1)cc,$($(1)archflags) $(filter %.o,$^) -o $@ $($(1)ldlibs))
endef
else
define compile_bin_o
	$(call compile_base,$(1)cxx,$($(1)archflags) $(filter %.o,$^) -o $@ $($(1)ldlibs))
endef
endif
