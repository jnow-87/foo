#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



subdir-y := unit/ kernel/


# test rules
.PHONY: test
test:
	$(foreach test,$^,$(call cmd_run_script,$(test)))
