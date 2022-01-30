#
# Copyright (C) 2022 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



.PHONY: test
test: all
	$(foreach test,$^, \
		$(call cmd_run_script, \
			[ ! -e $(test) ] || { \
				$(test) || { echo "$(test) failed"; exit 1; }; \
			} \
		) \
	)
