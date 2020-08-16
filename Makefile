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
src_dirs := sys arch kernel lib init testing scripts/memlayout scripts/arch scripts/devtree


####
## include build system Makefile
####

include $(scripts_dir)/main.make


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
	-Wno-unused-label \
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
	-Wno-unused-label \
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
	-Wno-unused-label \
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
	-Wno-unused-label \
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

kernel := $(build_tree)/kernel/kimg.elf
init := $(build_tree)/init/init.elf
libsys := $(build_tree)/lib/libsys.a
sysroot := $(build_tree)/sysroot
recent := recent

memlayout := $(build_tree)/scripts/memlayout/memlayout
sysroot_create := scripts/sysroot/create.sh


###################
###   targets   ###
###################

####
## build
####

# memlayout
.PHONY: memlayout
memlayout: $(memlayout)
	$(QUTIL)$(memlayout)

.PHONY: check_memlayout
check_memlayout: $(memlayout)
	$(call cmd_run_script,$(memlayout) --check)

.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
  cflags += -g
  cxxflags += -g
  asflags += -g
  ldlibs += -g
  hostcflags += -g -O0
  hostcxxflags += -g -O0
  hostasflags += -g
  hostldlibs += -g
endif

all: check_config check_memlayout $(lib) $(hostlib) $(bin) $(hostbin)
	$(call cmd_run_script, \
		@[ -e $(recent) ] && rm $(recent); \
		ln -s $(build_tree) $(recent); \
		[ ! $$? -eq 0 ] && echo -e '\033[31munable to create symbolic link "recent"\n\033[0m'; \
		$(sysroot_create) $(sysroot) $(patsubst <%>,%,$(CONFIG_ARCH_HEADER)) $(kernel) $(libsys) \
	)

####
## cleanup
####

.PHONY: clean
clean: clean-sysroot
	$(rm) $(filter-out $(patsubst %/,%,$(dir $(build_tree)/$(scripts_dir))),$(wildcard $(build_tree)/*)) $(recent)

.PHONY: clean-scripts
clean-scripts:
	$(rm) $(build_tree)/scripts

.PHONY: clean-sysroot
clean-sysroot:
	$(rm) $(sysroot)

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
