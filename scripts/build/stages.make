#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# check if prestage <stage> is currently executed
#
#	$(call is_prestage,<stage>)
define is_prestage
$(findstring $(1),$(PRESTAGE))
endef

# execute <cmd> during all stages except during <stages>
#
#	$(call skip_prestage,<stages>,<cmd>)
define skip_prestage
    $(if $(findstring $(PRESTAGE),$(1)),@:,$(2))
endef

# call make with the prestage target <stage-target>, setting the following
# environment variables to indicate prestage execution
# 	EXEC_PRESTAGE
# 	PRESTAGE
#
#	$(call prestage,<stage-name>,<targets>)
define prestage
  $(call pdebug0,prestage $(1) $(2)) \
  \
  $(eval r := $(shell \
      EXEC_PRESTAGE=$(prestage_key) \
      PRESTAGE=$(1) \
      $(MAKE) $(MFLAGS) $(MAKEOVERRIDES) $(2) 1>&2; \
      echo $$?
    ) \
  ) \
  \
  $(if $(findstring q,$(MFLAGS)),, \
    $(if $(filter-out 0,$(r)),$(error error while executing sub-make $(1) (targets: $(2))),) \
  )
endef

# invoke make prestages if it has not been done already
# use a key that identifies the local project, allowing to nest
# projects that use this set of makefiles
prestage_key := $(shell basename $$PWD)
skip_prestage_targets := help% \
	clean% %clean \
	install% uninstall% \
	menuconfig defconfig% \
	githooks check_coding_style check_build_tools check_user_tools

ifneq ($(EXEC_PRESTAGE),$(prestage_key))
  # ensure MAKECMDGOALS is not empty, otherwise the following checks would not work
  # and prestages would not be executed if no target is specified
  ifeq ($(MAKECMDGOALS),)
    MAKECMDGOALS := all
  endif

  # only call prestages if none of the following targets is called
  ifneq ($(filter-out $(skip_prestage_targets),$(MAKECMDGOALS)),)
    # only call prestages if make is not called by bash-completion
	# 	NOTE this check is only on best effort and can interfere with
	# 	     other invocations of make
    ifneq ($(MAKEFLAGS),npqw)
      # check if clean targets are mixed with other targets that require prestages to be executed
      ifneq ($(filter clean% %clean,$(MAKECMDGOALS)),)
        $(error Mixing clean with other targets is not supported by the build system)
      endif
  
      # stage0
      # 	required to update the *.cmd files
      # 	this cannot be done in a single run of make since that way variables, such as flags
      # 	that are passed via indirect dependencies are not know
      #
      # 	for instance
      # 		foo: cppflags-y += -DFOO
      # 		foo:
      # 			gcc $(cppflags-y)
      $(call prestage,stage0,prepare_deps $(MAKECMDGOALS))
  
      # stage1
      # 	required to build targets that for instance create makefiles
      # 	since those generate files might alter the build they cannot be build in a single
      # 	run of make
      $(call prestage,stage1,prepare_deps)
  
      $(call pdebug0,prestage done)
    endif
  endif
endif
