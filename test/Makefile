#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-$(if $(filter-out avr,$(CONFIG_ARCH)),y,) += \
	itest.elf \
	uttest.elf

hostbin-y := utest


# utest
utest-y := main.utest.o mock/ sys/ unit/ lib/
uttest-y := main.utest.o mock/ unit/

# target flags
uttest-cppflags := -Iinclude/lib

# ensure a rebuild if indirect dependencies are updated
uttest-y += $(build_tree)/lib/libbrick.a

main.utest-cppflags := -DTESTS=unit

ifeq ($(CONFIG_CODE_COVERAGE),y)
  uttest-$(CONFIG_X86) += $(build_tree)/arch/x86/gcov/libgcov.a
endif

uttest-ldlibs := \
	$(ldlibs-app) \
	-Tapp_$(CONFIG_ARCH).lds \
	-lbrick \
	-lgcc

# host flags
utest-hostcppflags := -I$(src_tree)/include
utest-hostcflags := -fno-builtin

main.utest-hostcppflags := $(main.utest-cppflags)


# itest
itest-y := main.itest.o user/ scripts/linker/app_$(CONFIG_ARCH).lds

# ensure a rebuild if indirect dependencies are updated
itest-y += $(build_tree)/lib/libbrick.a

ifeq ($(CONFIG_CODE_COVERAGE),y)
  itest-$(CONFIG_X86) += $(build_tree)/arch/x86/gcov/libgcov.a
endif

itest-cppflags := -Iinclude/lib

itest-ldlibs := \
	$(ldlibs-app) \
	-Tapp_$(CONFIG_ARCH).lds \
	-lbrick \
	-lgcc

main.itest-cppflags := -DTESTS=user_noninteractive


# custom rules to generate separate objects files for src files that are required by kernel and libbrick
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.utest.o,      $(loc_src_tree)/%.c)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.utest.host.o, $(loc_src_tree)/%.c, host)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.itest.o,      $(loc_src_tree)/%.c)


x86emu_bin := $(build_tree)/arch/x86/emu/x86emu

# execute the x86emu
#
#	$(call x86emu_exec,<app>)
define x86emu_exec
	$(call test_run,$@, \
		rootfs_base=$(CONFIG_X86EMU_FS_EXPORT_ROOT); \
		rootfs=$${rootfs_base}/$(subst x86emu-,,$@); \
		\
		mkdir -p $${rootfs_base}; \
		\
		echo "removing brickos file system export $${rootfs}"; \
		rm -rf $${rootfs}; \
		\
		$(x86emu_bin) --application $(1) --rootfs $${rootfs} \
	)
endef

# create x86emu rule
#
#	$(call x86emu,<rule-name>,<app>)
define x86emu
	$(call gen_rule_basic,x86emu_exec,x86emu-$(1),$(x86emu_bin),$(2))
endef


# test targets
test-$(if $(CONFIG_X86),n,y) := $(loc_build_tree)/utest

ifeq ($(CONFIG_X86),y)
  $(call x86emu,unit,$(loc_build_tree)/uttest.elf)
  $(call x86emu,user,$(loc_build_tree)/itest.elf)

  test-y += x86emu-unit x86emu-user
endif

test: $(test-y)
