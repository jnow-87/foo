#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



subdir-$(CONFIG_X86) += integration/

bin-y := itest.elf
hostbin-y := utest


# utest
utest-y := main.utest.o memory.o sys/ unit/

utest-hostcppflags += -I$(src_tree)/include
utest-hostcflags += -fno-builtin

main.utest-hostcppflags := -DTESTS=unit


# itest
itest-y := main.itest.o user/ scripts/linker/app_$(CONFIG_ARCH).lds

# ensure itest is rebuild if libs are updated and
# libs exist before building itest
itest-$(CONFIG_X86) += $(build_tree)/arch/x86/gcov/libgcov.a
itest-y += $(build_tree)/lib/libsys.a

itest-cppflags += \
	-I"include/lib" \
	-I"/include" \
	-I$(loc_build_tree)

itest-ldlibs += \
	$(ldlibs-app-arch) \
	-lsys \
	-lgcc \
	-Lscripts/linker \
	-Tapp_$(CONFIG_ARCH).lds

main.itest-cppflags := -DTESTS=user_noninteractive


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.utest.host.o, $(loc_src_tree)/%.c, host)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.itest.o,      $(loc_src_tree)/%.c)


# test targets
test-y := $(loc_build_tree)/utest
test-$(CONFIG_X86) += $(loc_src_tree)/integration/itest-prepare.sh
test-$(CONFIG_X86) += $(loc_build_tree)/integration/itestfw

test: $(test-y)
