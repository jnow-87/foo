#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# return basename of given path, also removing .host, if present
#
# 	$(call hostbasename,<file>)
define hostbasename
$(strip $(subst .host,,$(basename $(1))))
endef

# return files that do not define a separate list of dependencies,
# i.e. $(<file>-y) is empty
#
#	$(call filter_single_dep,<file list>
define filter_single_dep
	$(foreach f, $(1), \
		$(if $($(call hostbasename,$(f))-y), \
			, \
			$(f) \
		) \
	)
endef

# generate basic rule
#
#	$(call gen_rule,<cmd-name>,<target>,<dependencies>,<host-flag>
define gen_rule
	$(eval \
		$(call pdebug2,    generate rule:)
		$(call pdebug2,        $(strip $(2)): $(strip $(3)))
		$(if $(strip $(1)), \
			$(call pdebug2,            $(mkdir) $$(@D)) \
			$(call pdebug2,            $$(call $(strip $(1)),$(strip $(4)))) \
		)
		$(call pdebug2)

		$(if $(strip $(1)),
			$(eval \
				$(strip $(2)): $(strip $(3))
					$(mkdir) $$(@D)
					$$(call $(1),$(strip $(4))) \
			), \
			$(eval $(strip $(2)): $(strip $(3))) \
		) \
	)
endef

# generate basic rule handling command file creation
#
#	$(call gen_rule_basic,<cmd-name>,<target>,<dependencies>,<host-flag>
define gen_rule_basic
	$(if $(call cmd_file_required,$(1),$(2)), \
		$(if $(call is_prestage,stage0), \
			$(call gen_rule,$(1),$(2),$(3) force,$(4)) \
			, \
			$(call gen_rule,$(1),$(2),$(3) $(2).cmd,$(4)) \
		) \
		, \
		$(call gen_rule,$(1),$(2),$(3),$(4)) \
	)
endef

# generate target-specific rule for <target>-*flags and <target>-*flags-y
#	- generate: <target>: <flag> += <target>-<flag> <target>-<flag>-y
#	- generate: $(basename <target>).i: <flag> += <target>-<flag> <target>-<flag>-y
#		%.i files are only build on explicite request and thus do not depent on <target>
#
#	$(call gen_rule_tgt_flags,<target>,<flag>,<dir_prefix>
define gen_rule_tgt_flags
	$(if $($(call hostbasename,$(1))-$(2))$($(call hostbasename,$(1))-$(2)-y), \
		$(eval $(call gen_rule_basic,,$(3)$(1), $(2) += $($(call hostbasename,$(1))-$(2)) $($(call hostbasename,$(1))-$(2)-y))) \
		$(eval $(call gen_rule_basic,,$(3)$(basename $(1)).i, $(2) += $($(call hostbasename,$(1))-$(2)) $($(call hostbasename,$(1))-$(2)-y))) \
	)
endef

# generate target-specific rule for local flags, 
#	- generate: <target>: <flag> += <flag>-y subdir-<flag>
#	- generate: $(basename <target>).i: <flag> += <flag>-y subdir-<flag>
#		%.i files are only build on explicite request and thus do not depent on <target>
#
#	$(call gen_rule_flags,<target>,<flag>,<dir_prefix>
define gen_rule_loc_flags
	$(if $(strip $($(2)-y) $(loc_subdir-$(2))), \
		$(eval $(call gen_rule_basic,,$(3)$(1), $(2) += $($(2)-y) $(loc_subdir-$(2)))) \
		$(eval $(call gen_rule_basic,,$(3)$(basename $(1)).i, $(2) += $($(2)-y) $(loc_subdir-$(2)))) \
		, \
	)
endef

# generate rules for compound targets
# 	first $(<target>-y), which is $($(basename $(tgt))-y) is checked for external dependencies ($(ext_dep)), i.e. dependencies that
# 		origniate from a different path, are extracted
#
#	afterwards the rule is generated whereat <dir-prefix> is added to all non-external dependencies and external dependencies are
#		added as they are
#
#	$(call gen_rule_comp,<cmd-name>,<file list>,<dir_prefix>,<host-flag>
define gen_rule_comp
	$(foreach tgt,$(2), \
		$(eval ext_dep :=) \
		\
		$(foreach dep,$($(call hostbasename,$(tgt))-y), \
			$(if $(subst ./,,$(dir $(dep))), \
				$(eval ext_dep += $(dep)) \
				, \
			) \
		) \
		$(if $(strip $(4)), \
			$(call gen_rule_basic,$(1),$(3)$(tgt),$(patsubst %.o,%.host.o,$(addprefix $(3),$(filter-out $(ext_dep),$($(call hostbasename,$(tgt))-y))) $(ext_dep)),$(4)) \
			, \
			$(call gen_rule_basic,$(1),$(3)$(tgt),$(addprefix $(3),$(filter-out $(ext_dep),$($(call hostbasename,$(tgt))-y))) $(ext_dep),$(4)) \
		) \
	)
endef
