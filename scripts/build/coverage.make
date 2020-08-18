#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



cov_deps := check_user_tools check_build_tools $(coverage_script)
coverage: $(cov_deps)
	$(call cmd_run_script, \
		rm -f $$(find $(build_tree) -name '*.gcda'); \
		\
		for bin in $(filter-out $(cov_deps), $^); do \
			$${bin}; \
		done; \
		\
		gcda_files=$$(find $(build_tree) -name '*.gcda'); \
		\
		for file in $${gcda_files}; do \
			gcov -b $${file} > /dev/null; \
			tgt_name=$$(echo $${file} | sed -e 's/gcda$$/gcov/'); \
			mv *.gcov $${tgt_name}; \
		done; \
		\
		gcov_files=$$(echo $${gcda_files} | sed -e 's/\.gcda/.gcov/g'); \
		$(coverage_script) $${gcov_files}; \
	)
