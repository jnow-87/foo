#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



################
###   init   ###
################

####
## build system config
####


# build system variables
scripts_dir := scripts/build
project_type := c
config := .config
config_tree := scripts/config
use_config_sys := y
config_ftype := Kconfig
githooks_tree := .githooks

# source- and build-tree
default_build_tree := build/
src_dirs := arch kernel driver lib sys init testing scripts/memlayout scripts/arch scripts/devtree

kernel_name := kimg.elf
lib_name := libsys.a


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
	-Wall \
	-Wextra \
	-Wshadow \
	-Wno-unused-parameter \
	-Wno-unknown-pragmas \
	-nostdinc \
	-fno-builtin \
	-fshort-enums \
	-flto

cxxflags := \
	$(CXXFLAGS) \
	$(CONFIG_CXXFLAGS) \
	$(cxxflags) \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wno-unused-parameter \
	-Wno-unknown-pragmas \
	-nostdinc \
	-fno-builtin \
	-fshort-enums \
	-flto

cppflags := \
	$(CPPFLAGS) \
	$(CONFIG_CPPFLAGS) \
	$(cppflags) \
	-std=gnu99 \
	-I"$(src_tree)/include/" \
	-I"$(build_tree)/" \
	-DBUILD_ARCH_HEADER="$(CONFIG_ARCH_HEADER)"

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
	$(hostcflags) \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wno-unused-parameter \
	-Wno-unknown-pragmas \
	-O2 \
	-flto

hostcxxflags := \
	$(HOSTCXXFLAGS) \
	$(CONFIG_HOSTCCCFLAGS) \
	$(hostcxxflags) \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wno-unused-parameter \
	-Wno-unknown-pragmas \
	-O2 \
	-flto

hostcppflags := \
	$(HOSTCPPFLAGS) \
	$(CONFIG_HOSTCPPFLAGS) \
	$(hostcppflags) \
	-std=gnu99 \
	-I"$(build_tree)/" \
	-DBUILD_ARCH_HEADER="$(CONFIG_ARCH_HEADER)" \
	-DBUILD_HOST

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
## brickos specific variables
####

kernel := $(build_tree)/$(kernel_name)
kernel_objs := kernel/obj.o arch/obj.o driver/obj.o sys/obj.o
libsys := $(build_tree)/lib/$(lib_name)
libsys_objs := lib/obj.o sys/libsys.o arch/libsys.o

sysroot := sysroot
recent := recent

memlayout := $(build_tree)/scripts/memlayout/memlayout
memlayout_check := $(build_tree)/scripts/memlayout/memlayout_check
sysroot_create := scripts/sysroot/create.sh


###################
###   targets   ###
###################

####
## build
####

# kernel targets
kernel: cppflags += -DBUILD_KERNEL
kernel: check_config check_memlayout versionheader $(kernel)
kernel_deps: scripts/linker/kernel_$(CONFIG_ARCH).lds

$(kernel): ldlibs += $(ldlibs-kernel-arch)
$(kernel): ldlibs += -Lscripts/linker -Tkernel_$(CONFIG_ARCH).lds
$(kernel): ldlibs += -lgcc
$(kernel): kernel_deps $(addprefix $(build_tree)/, $(kernel_objs))
	$(call compile_bin_o)

# init targets
init: cppflags += -DBUILD_INIT
init: check_config libsys $(init)
init_deps: scripts/linker/app_$(CONFIG_ARCH).lds

$(init): ldlibs += -Lscripts/linker -Tapp_$(CONFIG_ARCH).lds
$(init): init_deps

# libsys targets
libsys: cppflags += -DBUILD_LIBSYS
libsys: check_config $(libsys)
libsys_deps:

$(libsys):
$(libsys): libsys_deps $(addprefix $(build_tree)/, $(libsys_objs))
	$(call compile_lib_o)

# sysroot targets
sysroot: kernel libsys init
	$(rm) $(recent)
	$(sym_link) $(build_tree) $(recent); test ! $$? -eq 0 && echo "\033[31munable to create symbolic link \"recent\",\n\033[0m"; exit 0
	$(QUTIL)$(sysroot_create) $(build_tree) $(sysroot) $(patsubst <%>,%,$(CONFIG_ARCH_HEADER)) $(kernel_name) $(lib_name)

# memlayout
.PHONY: memlayout
memlayout: $(memlayout)
	$(QUTIL)$(memlayout)

.PHONY: check_memlayout
check_memlayout: $(memlayout_check)
	$(QUTIL)$(memlayout_check)

.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
  cflags += -g
  cxxflags += -g
  asflags += -g
  ldlibs += -g
  hostcflags += -g
  hostcxxflags += -g
  hostasflags += -g
  hostldlibs += -g
endif

all: sysroot $(lib) $(hostlib) $(bin) $(hostbin)

####
## cleanup
####

.PHONY: clean
clean: clean-kernel clean-sysroot clean-init
	$(rm) $(filter-out $(patsubst %/,%,$(dir $(build_tree)/$(scripts_dir))),$(wildcard $(build_tree)/*))

.PHONY: clean-kernel
clean-kernel:
	$(rm) $(build_tree)/kernel $(build_tree)/driver $(build_tree)/sys $(build_tree)/lib $(build_tree)/arch $(kernel) $(recent)

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
	$(rm) $(config) $(config).old $(recent) $(build_tree)

####
## documentation
####
doxygen_config := doc/doxygen.conf
graphicspath := doc/img

svggraphics := $(shell find $(graphicspath) -name \*.svg)
pdfgraphics := $(patsubst %.svg, $(build_tree)/%.pdf, $(svggraphics))

.PHONY: docu
docu: fig_svg
	@sed -i -e 's>OUTPUT_DIRECTORY[ \t]*=.*>OUTPUT_DIRECTORY=$(build_tree)/doc/>' $(doxygen_config)
	@doxygen $(doxygen_config)
	@cp -r doc/* $(build_tree)/doc/latex
	@cp -r $(src_dirs) include $(build_tree)/doc/latex
	@cp -ru $(build_tree)/$(graphicspath) $(build_tree)/doc/latex/
	@make -C $(build_tree)/doc/latex
	$(echo) "\n\ndocumentation generated at $(build_tree)/doc"

.PHONY: fig_svg
fig_svg: fig_cp $(pdfgraphics)

.PHONY: fig_cp
fig_cp:
	$(mkdir) $(build_tree)/doc
	$(cp) -ru $(graphicspath) $(build_tree)/doc

$(pdfgraphics): %.pdf : %.svg
	inkscape -D -z -f $< --export-pdf=$@
