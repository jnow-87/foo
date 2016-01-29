################
###   init   ###
################

# init build system variables
scripts_dir := scripts
project_type := c
config := ./config
config_tree := $(scripts_dir)/config
use_config_sys := y
config_ftype := Kconfig

# include config
-include $(config)

# set parameter based on selected target
ifeq ($(CONFIG_P5020),y)
  platform := qoriq
  arch := powerpc
  board := p5020
  core := e5500
  arch_header := "<arch/$(arch)/board-$(board)/board.h>"
endif

ifeq ($(CONFIG_P4080),y)
  platform := qoriq
  arch := powerpc
  board := p4080
  core := e5500
  arch_header := "<arch/$(arch)/board-$(board)/board.h>"
endif

# init source and build tree
default_build_tree := build/
src_dirs := arch kernel lib sys init driver scripts/memlayout scripts/sysroot

# include build system Makefile
include $(scripts_dir)/Makefile.inc

# init default flags
cflags := $(CFLAGS) $(CONFIG_CFLAGS) -nostdinc -Wall -fno-builtin -Wno-unknown-pragmas
cxxflags := $(CXXFLAGS) $(CONFIG_CXXFLAGS)
cppflags := $(CPPFLAGS) $(CONFIG_CPPFLAGS) \
	-I"$(src_tree)/include/" \
	-DNCORES=$(CONFIG_NCORES) \
	-DCORE_MASK=$(CONFIG_CORE_MASK) \
	-DPLATFORM_$(call upper_case,$(platform)) \
	-DARCH_$(call upper_case,$(arch)) \
	-DBOARD_$(call upper_case,$(board)) \
	-DCORE_$(call upper_case,$(core)) \
	-DARCH_HEADER=$(arch_header) \
	-DSILICON_REV=$(CONFIG_SILICON_REV) \
	-DPLATFORM_CLOCK=$(CONFIG_PLATFORM_CLOCK) \
	-DCORE_CLOCK=$(CONFIG_CORE_CLOCK)

ldflags := $(LDFLAGS) $(CONFIG_LDFLAGS) -L"$(build_tree)/lib/" -nostartfiles -nostdlib -static
ldrflags := $(LDRFLAGS) $(CONFIG_LDRFLAGS)
asflags := $(ASFLAGS) $(CONFIG_ASFLAGS)
archflags := $(ARCHFLAGS) $(CONFIG_ARCHFLAGS)
hostcflags := $(HOSTCFLAGS) $(CONFIG_HOSTCFLAGS)
hostcxxflags := $(HOSTCXXFLAGS) $(CONFIG_HOSTCCCFLAGS)
hostcppflags := $(HOSTCPPFLAGS) $(CONFIG_HOSTCPPFLAGS) \
	-DNCORES=$(CONFIG_NCORES) \
	-DPLATFORM_$(call upper_case,$(platform)) \
	-DARCH_$(call upper_case,$(arch)) \
	-DBOARD_$(call upper_case,$(board)) \
	-DCORE_$(call upper_case,$(core)) \
	-DARCH_HEADER=$(arch_header)

hostldflags := $(HOSTLDFLAGS) $(CONFIG_HOSTLDFLAGS)
hostldrflags := $(HOSTLDRFLAGS) $(CONFIG_HOSTLDRFLAGS)
hostasflags := $(HOSTASFLAGS) $(CONFIG_HOSTASFLAGS)
hostarchflags := $(HOSTARCHFLAGS) $(CONFIG_HOSTARCHFLAGS)
yaccflags := $(YACCFLAGS)
lexflags := $(LEXFLAGS)
gperfflags := $(GPERFFLAGS)

# init build variables
LAST_BUILD ?= last_build
last_build := $(LAST_BUILD)

SYSROOT ?= sysroot
sysroot := $(SYSROOT)

memlayout := $(build_tree)/scripts/memlayout/memlayout
macro_replacer := $(build_tree)/scripts/sysroot/macro_replacer
kernel := $(build_tree)/kimg.elf
kernel_obj := $(addprefix $(build_tree)/,kernel/obj.o arch/obj.o sys/obj.o driver/obj.o)
libsys := $(build_tree)/lib/libsys.a
libsys_obj := $(addprefix $(build_tree)/,lib/obj.o sys/obj.o arch/for_libsys.o)


###################
###   targets   ###
###################

####
## build
####
# kernel target
.PHONY: kernel
kernel: check_config $(memlayout) $(kernel)
	@$(memlayout)

$(kernel): ldflags += -L scripts/linker -T kernel_$(arch).lds -lgcc
$(kernel): $(kernel_obj)
	$(call compile_bin_o)

# libsys targets
.PHONY: libsys
libsys: check_config $(libsys)

$(libsys): ldflags += -L scripts/linker -T libsys_$(arch).lds
$(libsys): $(libsys_obj)
	$(call compile_lib_o)

.PHONY: libsys_dep
libsys_dep:
	$(echo) $(libsys_obj)

# sysroot target
.PHONY: sysroot
sysroot: check_config $(macro_replacer) $(kernel) $(libsys)
	$(rm) $(last_build)
	$(sym_link) $(build_tree) $(last_build); test ! $$? -eq 0 && echo "\033[31munable to create symbolic link \"last_build\",\nmake sure to update &build_dir within lauterbach/setup_env.cmm and\nthe path to init.elf within lauterbach/apps.txt\033[0m"; exit 0
	@scripts/sysroot/create.sh $(build_tree) $(sysroot) $(arch)

.PHONY: all
all: kernel libsys sysroot $(lib) $(hostlib) $(bin) $(hostbin)

####
## cleanup
####
.PHONY: clean
clean: clean-kernel clean-init clean-scripts
	$(rm) $(filter-out $(build_tree)/$(scripts_dir),$(wildcard $(build_tree)/*)) $(last_build)

.PHONY: clean-kernel
clean-kernel:
	$(rm) $(build_tree)/kernel $(build_tree)/driver $(build_tree)/sys $(build_tree)/lib $(build_tree)/arch $(kernel) $(last_build)

.PHONY: clean-init
clean-init:
	$(rm) $(build_tree)/init

.PHONY: clean-scripts
clean-scripts:
	$(rm) $(build_tree)/scripts

.PHONY: clean-sysroot
clean-sysroot:
	$(rm) $(build_tree)/$(sysroot)

.PHONY: distclean
distclean:
	$(rm) config config.old $(last_build) $(build_tree)
