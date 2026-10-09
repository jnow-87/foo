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
			stem=$$(basename $${file} | cut -d '.' -f 1); \
			tgt_name=$$(echo $${file} | sed -e 's/gcda$$/gcov/'); \
			\
			gcov -b $${file} > /dev/null; \
			gcov_name=$$(ls $${stem}*.gcov | tail -1); \
			\
			mv $${gcov_name} $${tgt_name} || exit 1; \
			rm -f *.gcov; \
		done; \
		\
		[ "$${gcda_files}" != "" ] || { echo $(call fg,red,"error")": no coverage data found"; exit 1; }; \
		gcovered $(gcovered_args) $(build_tree); \
	)

.PHONY: coverage-clean
coverage-clean:
	$(call cmd_run_script, rm -f $$(find $(build_tree) -name '*.gcda'))

test: coverage-clean
