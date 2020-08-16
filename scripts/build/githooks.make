#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# ensure githook links exists
.PHONY: githooks
.SILENT: githooks
githooks:
	$(call cmd_run_script, \
		repo_root=$$(git rev-parse --show-toplevel); \
		\
		for hook in style mantis util_print pre-commit post-commit; do \
		  test -e "$(githooks_tree)/$${hook}" || { echo $(githooks_tree)/$${hook} does not exist; exit 1; }; \
		  ln -frs $(githooks_tree)/$${hook} $${repo_root}/.git/hooks/$${hook}; \
		done \
	)

# check coding style
.PHONY: check_coding_style
check_coding_style:
	$(call cmd_run_script,${githooks_tree}/pre-commit $(shell find -type f | grep -v -e '\./\.git/' -e '\./build/'))

all: githooks
