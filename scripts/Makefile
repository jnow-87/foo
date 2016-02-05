################
###   init   ###
################

####
## build system config
####

# build system variables
scripts_dir := scripts
project_type := c
config := ./config
config_tree := $(scripts_dir)/config
use_config_sys := y
config_ftype := Kconfig

# source- and build-tree
default_build_tree := build/
src_dirs := arch kernel lib sys init driver $(scripts_dir)/memlayout

####
## include build system Makefile
####

include $(scripts_dir)/Makefile.inc


####
## flags
####

# init default target flags
cflags := \
	$(CFLAGS) \
	$(CONFIG_CFLAGS) \
	$(cflags) \
	-nostdinc \
	-Wall \
	-Wno-unknown-pragmas \
	-fno-builtin

cxxflags := \
	$(CXXFLAGS) \
	$(CONFIG_CXXFLAGS) \
	$(cxxflags)

cppflags := \
	$(CPPFLAGS) \
	$(CONFIG_CPPFLAGS) \
	$(cppflags) \
	-I"$(src_tree)/include/" \
	-I"$(build_tree)/" \
	-DARCH_HEADER="$(CONFIG_ARCH_HEADER)"

ldlibs := \
	$(LDLIBS) \
	$(CONFIG_LDLIBS) \
	$(ldlibs) \
	-nostartfiles \
	-nostdlib \
	-static \
	-L$(build_tree)/lib/

ldflags := \
	$(LDFLAGS) \
	$(CONFIG_LDFLAGS) \
	$(ldflags)

asflags := \
	$(ASFLAGS) \
	$(CONFIG_ASFLAGS) \
	$(asflags)

archflags := \
	$(ARCHFLAGS) \
	$(CONFIG_ARCHFLAGS) \
	$(archflags)

# init default host flags
hostcflags := \
	$(HOSTCFLAGS) \
	$(CONFIG_HOSTCFLAGS) \
	$(hostcflags)

hostcxxflags := \
	$(HOSTCXXFLAGS) \
	$(CONFIG_HOSTCCCFLAGS) \
	$(hostcxxflags)

hostcppflags := \
	$(HOSTCPPFLAGS) \
	$(CONFIG_HOSTCPPFLAGS) \
	$(hostcppflags) \
	-I"$(build_tree)/" \
	-DARCH_HEADER="$(CONFIG_ARCH_HEADER)"

hostldlibs := \
	$(HOSTLDLIBS) \
	$(CONFIG_HOSTLDLIBS) \
	$(hostldlibs)

hostldflags := \
	$(HOSTLDFLAGS) \
	$(CONFIG_HOSTLDFLAGS) \
	$(hostldflags)

hostasflags := \
	$(HOSTASFLAGS) \
	$(CONFIG_HOSTASFLAGS) \
	$(hostasflags)

hostarchflags := \
	$(HOSTARCHFLAGS) \
	$(CONFIG_HOSTARCHFLAGS) \
	$(hostarchflags)

yaccflags := \
	$(YACCFLAGS) \
	$(yaccflags)

lexflags := \
	$(LEXFLAGS) \
	$(lexflags)

gperfflags := \
	$(GPERFFLAGS) \
	$(gperfflags)


####
## janos specific variables
####

kernel := $(build_tree)/kimg.elf
kernel_deps := kernel/ arch/ sys/ driver/
libsys := $(build_tree)/lib/libsys.a
libsys_dep := lib/obj.o sys/obj.o arch/libsys.o

sysroot := sysroot
recent := recent

memlayout := $(build_tree)/$(scripts_dir)/memlayout/memlayout
sysroot_create := $(scripts_dir)/sysroot/create.sh


###################
###   targets   ###
###################

####
## build
####

# kernel target
.PHONY: kernel
kernel: cppflags += -DKERNEL
kernel: check_config $(memlayout) $(kernel)
	$(QUTIL)$(memlayout)

$(kernel): ldlibs += -L$(scripts_dir)/linker -Tkernel_$(CONFIG_ARCH).lds
$(kernel): ldlibs += -Wl,--section-start=.base=$(CONFIG_KERNEL_BASE_ADDR)
$(kernel): ldlibs += -lgcc
$(kernel): $(addsuffix obj.o, $(addprefix $(build_tree)/, $(kernel_deps)))
	$(call compile_bin_o)

# libsys targets
.PHONY: libsys
libsys: cppflags += -DLIBSYS
libsys: check_config $(libsys)

$(libsys): ldlibs += -L$(scripts_dir)/linker -Tlibsys_$(CONFIG_ARCH).lds
$(libsys): $(addprefix $(build_tree)/, $(libsys_dep))
	$(call compile_lib_o)

# sysroot target
.PHONY: sysroot
sysroot: check_config $(kernel) $(libsys)
	$(rm) $(recent)
	$(sym_link) $(build_tree) $(recent); test ! $$? -eq 0 && echo "\033[31munable to create symbolic link \"recent\",\n\033[0m"; exit 0
	$(QUTIL)$(sysroot_create) $(build_tree) $(sysroot) $(patsubst <%>,%,$(CONFIG_ARCH_HEADER)) $(build_tree)/config/config.h

.PHONY: all
all: kernel libsys sysroot $(lib) $(hostlib) $(bin) $(hostbin)

####
## cleanup
####

.PHONY: clean
clean: clean-kernel clean-init clean-scripts
	$(rm) $(filter-out $(build_tree)/$(scripts_dir),$(wildcard $(build_tree)/*))

.PHONY: clean-kernel
clean-kernel:
	$(rm) $(build_tree)/kernel $(build_tree)/driver $(build_tree)/sys $(build_tree)/lib $(build_tree)/arch $(kernel) $(recent)

.PHONY: clean-init
clean-init:
	$(rm) $(build_tree)/init

.PHONY: clean-scripts
clean-scripts:
	$(rm) $(build_tree)/$(scripts_dir)

.PHONY: clean-sysroot
clean-sysroot:
	$(rm) $(build_tree)/$(sysroot)

.PHONY: distclean
distclean:
	$(rm) config config.old $(recent) $(build_tree)
