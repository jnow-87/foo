#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



all_user_tools += gcovered

gcovered_args := 

ifneq ("$(gcovered_rc)","")
  gcovered_args += -r $(gcovered_rc)
endif


.PHONY: coverage
coverage: test
	$(call cmd_run_script, \
		gcda_files=$$(find $(build_tree) -name '*.gcda'); \
		\
		for file in $${gcda_files}; do \
			gcov -b $${file} > /dev/null; \
			tgt_name=$$(echo $${file} | sed -e 's/gcda$$/gcov/'); \
			mv *.gcov $${tgt_name}; \
		done; \
		\
		[ "$${gcda_files}" != "" ] || { echo $(call fg,red,"error")": no coverage data found"; exit 1; }; \
		gcovered $(gcovered_args) $(build_tree); \
	)

.PHONY: coverage-clean
coverage-clean:
	$(call cmd_run_script, rm -f $$(find $(build_tree) -name '*.gcda'))

test: coverage-clean
