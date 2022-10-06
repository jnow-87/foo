#
# Copyright (C) 2015 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



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
		$(QBUILD)$(1)
	)
endef

# generate a dependency file for the current target
# 	dependency file is not generated while executing prestage stage0
# 	fixdep is only applied if configtools are configured to be used
# 	and built already
#
#	$(call gen_deps,<compiler>,<compile-flags>)
define gen_deps
	$(call skip_prestage,stage0,
		$($(1)) $(filter-out %.cmd,$(2)) -MM -MF $@.d -MP -MT $@ $<
		$(if $(configtools_unavailable), \
			,
			$(mv) $@.d $@.d.tmp
			$(fixdep) $@.d.tmp $(config_header) $(dir $(config_header))fixdep/ 1> $@.d
		) \
	)
endef

# apply the compiler's pre-processor to $< during
# all stages except stage0
#
#	$(call preproc_file
define preproc_file
	$(call skip_prestage,stage0,
		$(echo) [PREPROC] $@ $(if $(WHATCHANGED),\($?\))
		$(cc) $(cppflags) -xc -E -P $< -o $@
	)
	$(call gen_deps,cc,$(cppflags) -xc)
endef

define cmd_defconfig
	$(echo) [CP] $< '->' $(config)
	@(test -e $(config) && cp $(config) $(config).old) ; exit 0
	$(cp) $< $(config)
endef
