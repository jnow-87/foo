#
# Copyright (C) 2022 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# execute a test
#
#	$(call test_run,<test-name>,<script>)
define test_run
	$(call cmd_run_script, $(2) && { echo "test $(1)..." $(call fg,green,"passed"); } || { echo "test $(1)..." $(call fg,red,"failed"); exit 1; })
endef


.PHONY: test
test: all
	$(foreach test,$^, \
		$(call cmd_run_script, \
			$(if $(wildcard $(test)), \
				$(call test_run,$(test),$(test)) \
			) \
		) \
	)
