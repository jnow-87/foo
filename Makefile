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
src_dirs := arch kernel lib sys init testing scripts/memlayout

kernel_name := kimg.elf
lib_name := libsys.a
init_name := init.elf


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
	$(hostcflags) \
	-Wall \
	-Wno-unknown-pragmas \
	-O2 \
	-flto

hostcxxflags := \
	$(HOSTCXXFLAGS) \
	$(CONFIG_HOSTCCCFLAGS) \
	$(hostcxxflags) \
	-Wall \
	-Wno-unknown-pragmas \
	-O2 \
	-flto

hostcppflags := \
	$(HOSTCPPFLAGS) \
	$(CONFIG_HOSTCPPFLAGS) \
	$(hostcppflags) \
	-std=gnu99 \
	-I"$(build_tree)/" \
	-DARCH_HEADER="$(CONFIG_ARCH_HEADER)" \
	-DHOST

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
kernel_deps := kernel/obj.o arch/obj.o sys/obj.o
libsys := $(build_tree)/lib/$(lib_name)
libsys_dep := lib/obj.o sys/obj.o arch/libsys.o
init := $(build_tree)/$(init_name)
init_deps := init/obj.o

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

# kernel target
.PHONY: kernel
kernel: cppflags += -DKERNEL
kernel: check_config check_configheader check_memlayout versionheader $(kernel)

$(kernel): ldlibs += $(ldlibs-kernel-arch)
$(kernel): ldlibs += -Lscripts/linker -Tkernel_$(CONFIG_ARCH).lds
$(kernel): ldlibs += -lgcc
$(kernel): $(addprefix $(build_tree)/, $(kernel_deps))
	$(call compile_bin_o)

# libsys targets
.PHONY: libsys
libsys: cppflags += -DLIBSYS
libsys: check_config check_configheader $(libsys)

$(libsys): ldlibs += -Lscripts/linker -Tlibsys_$(CONFIG_ARCH).lds
$(libsys): $(addprefix $(build_tree)/, $(libsys_dep))
	$(call compile_lib_o)

# init targets
.PHONY: init
init: cppflags += -DINIT
init: check_config check_configheader libsys $(init)

$(init): ldlibs += -Lscripts/linker -Tinit_$(CONFIG_ARCH).lds
$(init): ldlibs += -Wl,--section-start=.data=0x800400
$(init): ldlibs += -lsys -lgcc
$(init): $(addprefix $(build_tree)/, $(init_deps))
	$(call compile_bin_o)

# sysroot target
.PHONY: sysroot
sysroot: kernel libsys
	$(rm) $(recent)
	$(sym_link) $(build_tree) $(recent); test ! $$? -eq 0 && echo "\033[31munable to create symbolic link \"recent\",\n\033[0m"; exit 0
	$(QUTIL)$(sysroot_create) $(build_tree) $(sysroot) $(patsubst <%>,%,$(CONFIG_ARCH_HEADER)) $(build_tree)/config/config.h $(kernel_name) $(lib_name)

# memlayout
.PHONY: memlayout
memlayout: check_configheader $(memlayout)
	$(QUTIL)$(memlayout)

.PHONY: check_memlayout
check_memlayout: check_configheader $(memlayout_check)
	$(QUTIL)$(memlayout_check)

.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
cflags += -g
cxxflags += -g
asflags += -g
hostcflags += -g
hostcxxflags += -g
hostasflags += -g
endif

all: kernel libsys sysroot $(lib) $(hostlib) $(bin) $(hostbin)

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


documentation: fig_svg
	@sed -i -e 's>OUTPUT_DIRECTORY[ \t]*=.*>OUTPUT_DIRECTORY=$(build_tree)/doc/>' $(doxygen_config)
	@doxygen $(doxygen_config)
	@cp -r doc/* $(build_tree)/doc/latex
	@cp -r $(src_dirs) include $(build_tree)/doc/latex
	@cp -ru $(build_tree)/$(graphicspath) $(build_tree)/doc/latex/
	@make -C $(build_tree)/doc/latex
	$(echo) "\n\ndocumentation generated at $(build_tree)/doc"

.PHONY:	fig_svg
fig_svg: fig_cp $(pdfgraphics)

.PHONY: fig_cp
fig_cp:
	@cp -ru $(graphicspath) $(build_tree)/doc

$(pdfgraphics): %.pdf : %.svg
	inkscape -D -z -f $< --export-pdf=$@
