#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



subdir-$(CONFIG_X86) += integration/

hostbin-y := utest

utest-y := main.utest.o log.o memory.o sys/ unit/


utest-hostcppflags += -I$(src_tree)/include
utest-hostcflags += -fno-builtin

main.utest-hostcppflags := -DTESTS=unit


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.utest.host.o,    $(loc_src_tree)/%.c, host)


# test rules
.PHONY: test
test:
	$(foreach test,$^,$(call cmd_run_script,$(test)))

test: $(loc_build_tree)/utest
coverage: $(loc_build_tree)/utest
