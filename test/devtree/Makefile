#
# Copyright (C) 2024 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := dttest

# test devtree files
dts := $(loc_src_tree)/test.dts
dt_preproc := $(loc_build_tree)/test.i.dts
dt_make := $(dt_preproc).make
dt_header := $(dt_preproc).h
dt_src := $(dt_preproc).c

# dttest
dttest-y := main.o $(dt_preproc).o $(build_tree)/sys/devtree.o

hostcppflags-y := -I$(src_tree)/include -I$(loc_build_tree)
hostcflags-y := -fno-builtin

$(loc_build_tree)/main.host.o: $(dt_header)

# devtree generation
$(call gen_rule_basic, compile_o_c, $(dt_preproc).host.o, $(dt_src), host)

$(dt_preproc): $(dts) $(dtc)
	$(call preproc_file)

$(dt_src): $(dt_preproc)
	$(call cmd_run_script,$(dtc) --format=c --sections=all -o $<.c $<)

$(dt_make): $(dt_preproc)
	$(call cmd_run_script,$(dtc) --format=make --sections=all -o $<.make $<)

$(dt_header): $(dt_preproc)
	$(call cmd_run_script,$(dtc) --format=header --sections=all -o $<.h $<)

# test targets
test-devtree: $(dt_preproc) $(dt_make) $(dt_header)
test: test-devtree $(loc_build_tree)/dttest
