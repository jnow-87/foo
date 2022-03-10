#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



subdir-$(CONFIG_X86) += integration/

bin-y := itest.elf
bin-$(if $(filter-out avr,$(CONFIG_ARCH)),y,) += uttest.elf
hostbin-y := utest


# utest
utest-y := main.utest.o mock/ sys/ unit/
uttest-y := main.utest.o mock/ unit/

# target flags
uttest-cppflags := -I"include/lib"

main.utest-cppflags := -DTESTS=unit

# ensure itest is rebuild if libs are updated and
# libs exist before building itest
uttest-y += $(build_tree)/lib/libsys.a

ifeq ($(CONFIG_CODE_COVERAGE),y)
  uttest-$(CONFIG_X86) += $(build_tree)/arch/x86/gcov/libgcov.a
endif

uttest-ldlibs := \
	$(ldlibs-app) \
	-Tapp_$(CONFIG_ARCH).lds \
	-lsys \
	-lgcc

# host flags
utest-hostcppflags := -I$(src_tree)/include
utest-hostcflags := -fno-builtin

main.utest-hostcppflags := $(main.utest-cppflags)


# itest
itest-y := main.itest.o user/ scripts/linker/app_$(CONFIG_ARCH).lds

# ensure itest is rebuild if libs are updated and
# libs exist before building itest
itest-y += $(build_tree)/lib/libsys.a

ifeq ($(CONFIG_CODE_COVERAGE),y)
  itest-$(CONFIG_X86) += $(build_tree)/arch/x86/gcov/libgcov.a
endif

itest-cppflags := -I"include/lib"

itest-ldlibs := \
	$(ldlibs-app) \
	-Tapp_$(CONFIG_ARCH).lds \
	-lsys \
	-lgcc

main.itest-cppflags := -DTESTS=user_noninteractive


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.utest.o,      $(loc_src_tree)/%.c)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.utest.host.o, $(loc_src_tree)/%.c, host)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.itest.o,      $(loc_src_tree)/%.c)


# test targets
test-y := $(loc_build_tree)/utest
test-$(CONFIG_X86) += $(loc_src_tree)/integration/itest-prepare.sh
test-$(CONFIG_X86) += $(loc_build_tree)/integration/itestfw

test: $(test-y)
