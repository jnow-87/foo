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
use_coverage_sys = $(CONFIG_CODE_COVERAGE)
gcovered_rc := .gcoveredrc

# source- and build-tree
default_build_tree := build/
src_dirs := \
	scripts/ \
	sys/ \
	arch/ \
	kernel/ \
	lib/ \
	init/ \
	test/


####
## flags (init values)
####

ldlibs-kernel :=
ldlibs-app :=


####
## include build system Makefile
####

include $(scripts_dir)/main.make


####
## flags
####

# warning flags
warnflags := \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wno-unused-parameter \
	-Wno-unused-label \
	-Wno-unknown-pragmas

# target flags
cflags := \
	$(CFLAGS) \
	$(CONFIG_CFLAGS) \
	$(cflags) \
	$(warnflags) \
	-nostdinc \
	-fno-builtin \
	-fshort-enums \
	-fsigned-char

cxxflags := \
	$(CXXFLAGS) \
	$(CONFIG_CXXFLAGS) \
	$(cxxflags) \
	$(warnflags) \
	-nostdinc \
	-fno-builtin \
	-fshort-enums \
	-fsigned-char

cppflags := \
	$(CPPFLAGS) \
	$(CONFIG_CPPFLAGS) \
	$(cppflags) \
	-std=gnu99 \
	-I$(src_tree)/include \
	-I$(build_tree) \
	-DBUILD_ARCH_HEADER="$(CONFIG_ARCH_HEADER)"

ldlibs := \
	$(LDLIBS) \
	$(CONFIG_LDLIBS) \
	$(ldlibs) \
	-nostartfiles \
	-nostdlib \
	-static \
	-L$(build_tree)/lib/ \
	-Lscripts/linker \
	-L$(build_tree)/scripts/linker

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

# host flags
hostcflags := \
	$(HOSTCFLAGS) \
	$(CONFIG_HOSTCFLAGS) \
	$(hostcflags) \
	$(warnflags) \
	-O2 \
	-flto=auto

hostcxxflags := \
	$(HOSTCXXFLAGS) \
	$(CONFIG_HOSTCXXFLAGS) \
	$(hostcxxflags) \
	$(warnflags) \
	-O2 \
	-flto=auto

hostcppflags := \
	$(HOSTCPPFLAGS) \
	$(CONFIG_HOSTCPPFLAGS) \
	$(hostcppflags) \
	-std=gnu99 \
	-I$(build_tree) \
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

# coverage flags
ifeq ($(CONFIG_CODE_COVERAGE),y)
  cflags += -fprofile-arcs -ftest-coverage --coverage
  cxxflags += -fprofile-arcs -ftest-coverage --coverage
  ldlibs += -lgcov
endif


####
## brickos specific variables
####

kernel := $(build_tree)/kernel/kimg.elf
init := $(build_tree)/init/init.elf
libbrick := $(build_tree)/lib/libbrick.a
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
	$(call cmd_run_script,$(memlayout))

.PHONY: check_memlayout
check_memlayout: $(memlayout)
	$(call cmd_run_script,$(memlayout) --check)

.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
  cflags += -g
  cxxflags += -g
  ldlibs += -g
  hostcflags += -g -Og
  hostcxxflags += -g -Og
  hostasflags += -g
  hostldlibs += -g

  ifneq ($(CONFIG_ARCH),avr)
  	# on avr, if assembly is compiled through gcc instead of as and -g is
	# used, binutils report an error during a later incremental link
    asflags += -g
  endif
endif

all: check_config check_memlayout $(lib) $(hostlib) $(bin) $(hostbin)
	$(call cmd_run_script, \
		@[ -e $(recent) ] && rm $(recent); \
		ln -sf $(build_tree) $(recent); \
		[ ! $$? -eq 0 ] && echo $(call fg,red,"error")": unable to create symbolic link" $(call fg,violet,"recent"); \
		$(sysroot_create) $(sysroot) $(patsubst <%>,%,$(CONFIG_ARCH_HEADER)) $(kernel) $(libbrick) \
	)

####
## coverage
####

ifneq ($(CONFIG_CODE_COVERAGE),y)
.PHONY: coverage
coverage:
	$(call cmd_run_script, echo "code coverage has been disabled through" $(call fg,yellow,"CONFIG_CODE_COVERAGE"))
endif

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

manual := $(build_tree)/doc/brickos.pdf

tex_tree := doc/latex
tex_build_tree := $(build_tree)/$(tex_tree)
tex_src := $(shell find doc/ $(tex_build_tree) -name \*.tex 2>/dev/null)

doxygen_tex := $(tex_build_tree)/refman.tex
doxygen_pdf := $(tex_build_tree)/refman.pdf
doxygen_log := $(build_tree)/doc/doxygen.log
doxygen_src_dirs := include arch driver init kernel lib sys
doxygen_src_files := $(shell find $(doxygen_src_dirs) -name \*.[hc])

doxygen_config_src := doc/doxygen.conf
doxygen_config_tgt := $(build_tree)/doc/doxygen.conf

graphicspath := doc/img
svggraphics := $(shell find $(graphicspath) -name \*.svg)
pdfgraphics := $(patsubst doc/%.svg, $(tex_build_tree)/%.pdf, $(svggraphics))

.PHONY: docu
docu: $(manual)
	$(call cmd_run_script, $(echo) "documentation:" $(call fg,violet,$<))

$(manual): $(doxygen_pdf)
	$(call cmd_run_script, $(cp) $< $@)

$(doxygen_pdf): $(pdfgraphics) $(doxygen_tex) $(tex_src)
	$(call cmd_run_script, $(cp) -ru doc/* $(tex_build_tree))
	$(call cmd_run_script, $(cp) -ru $(doxygen_src_dirs) $(tex_build_tree))
	$(call cmd_run_script, $(echo) "executing pdflatex (log:" $(call fg,violet,"$(tex_build_tree)/pdflatex.log")")...")
	$(call cmd_run_script, \
		cd $(tex_build_tree) \
		&& pdflatex -interaction=nonstopmode refman 1>pdflatex.log 2>&1 \
		&& makeindex refman.idx 1>>pdflatex.log 2>&1 \
		&& pdflatex -interaction=nonstopmode refman 1>>pdflatex.log 2>&1 \
	)

$(doxygen_tex): $(doxygen_config_tgt) $(doxygen_src_files)
	$(call cmd_run_script, $(echo) "executing doxygen (log:" $(call fg,violet,"$(doxygen_log)")")...")
	$(call cmd_run_script, doxygen $(doxygen_config_tgt) 1>$(doxygen_log) 2>&1)

$(doxygen_config_tgt): $(doxygen_config_src)
	$(call cmd_run_script, $(mkdir) $(dir $@))
	$(call cmd_run_script, $(cp) -ru $< $@)
	$(call cmd_run_script, sed -i -e "s:OUTPUT_DIRECTORY[ \t]*=.*:OUTPUT_DIRECTORY=$(build_tree)/doc/:" $@)
	$(call cmd_run_script, sed -i -e "s:INPUT[ \t]*=.*:INPUT=$(doxygen_src_dirs):" $@)

$(tex_build_tree)/%.pdf: doc/%.svg
	$(call cmd_run_script, $(mkdir) $(dir $@))
	$(call cmd_run_script, inkscape --export-area-drawing --export-filename=$@ $<)
