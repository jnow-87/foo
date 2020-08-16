#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# recursively include 'build.make' on given directories, avoiding
# double-inclusion of the same Makefile
# directory for current iteration is available through $(loc_dir)
#
#	$(call dinclude,<directory list>)
define dinclude
	$(eval traverse := $(call unique,$(filter-out $(included),$(patsubst %/,%,$(1))))) \
	$(eval included += $(traverse)) \
	\
	$(foreach dir,$(traverse), \
		$(eval loc_dir=$(dir)) \
		$(eval include $(build)) \
	)
endef
