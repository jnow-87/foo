#
# Copyright (C) 2023 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



PREFIX ?= ~/bin


# install the given file to <target> or $(PREFIX)
# if <target> is not defined
#
#	$(call install,<file>[, <target>])
define install
	$(eval tgt := $(if $2,$2,$(PREFIX)))
	$(mkdir) -p $(tgt)
	$(echo) "[INSTALL] $1 -> $(tgt)"
	$(sym_link) -rf $1 $(tgt)
endef

# uninstall the given file to $(PREFIX)
#
#	$(call uninstall,<file>)
define uninstall
	$(echo) [RM] $1
	$(rm) $1
endef	
