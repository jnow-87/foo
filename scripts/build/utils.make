#
# Copyright (C) 2015 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# variables that contain ' ' and ',' required for some replacements, that
# do not work with the characters used literally 
space := $(subst ,, )
comma := ,

# check if a file exists
#
# 	$(call exists,<file>)
define exists
$(if $(wildcard $(1)),1,)
endef

# convert string to upper case
#
# 	$(call upper_case,<string>)
define upper_case
$(shell echo $(1) | tr a-z A-Z)
endef

# returns 1 if $1 >= $2
#
# 	$(call cond_ge,<a>,<b>)
define cond_ge
$(shell test $1 -ge $2 && echo 1)
endef

# remove duplicates from string
#
# 	$(strip $(call unique,<string>))
define unique
	$(eval lst :=) \
	$(foreach k,$(1),$(if $(filter $(k), $(lst)),,$(eval lst += $(k)))) \
	$(lst)
endef


# set default value of variable <var> based on the following scheme
#	if <var> is defined on the command-line
#		do not change it
#	else if CONFIG_<var> is defined
#		<var> := CONFIG_<var>
#	else
#		<var> := <value>
#
# $(call set_default,<var>,<value>)
define set_default
	$(if $(subst environment,,$(subst command line,,$(origin $(1)))), \
		$(if $(CONFIG_$(1)), \
			$(eval $(1) := $(CONFIG_$(1))) \
			, \
			$(eval $(1) := $(2)) \
		) \
	)
endef
