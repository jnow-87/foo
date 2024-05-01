#
# Copyright (C) 2019 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := devicetree.o
hostobj-y := devicetree.o
hostbin-y := dtc

dtc-y := main.o options.o export.o nodes.o asserts.o lexer.lex.o parser.tab.o

# ensure a rebuild if indirect dependencies are updated
dtc-y += $(build_tree)/sys/libhost.a

# additional dependencies for generated headers
$(loc_build_tree)/parser.tab.host.o: $(loc_build_tree)/lexer.lex.host.o
$(loc_build_tree)/lexer.lex.host.o: $(loc_build_tree)/parser.tab.c
$(loc_build_tree)/main.host.o: $(loc_build_tree)/parser.tab.c


hostcppflags-y := \
	-I$(src_tree) \
	-I$(src_tree)/include \
	-I$(loc_src_tree) \
	-I$(loc_build_tree) \
	-I/usr/include

hostcflags-y := \
	-fno-builtin

hostldlibs-y := \
	-L$(build_tree)/sys -lhost


####
## device tree generation
####

dtc := $(loc_build_tree)/dtc

devtree_path := $(build_tree)/sys
devtree_preproc := $(devtree_path)/devicetree.i.dts
devtree_make := $(devtree_path)/devicetree.make
devtree_header := $(devtree_path)/devicetree.h
devtree_src := $(devtree_path)/devicetree.c

prepare_deps: $(devtree_header) $(devtree_make) $(devtree_src)

# compile the desired devicetree <output-format>
#
#	$(call compile_devicetree,<output-format>,<sections>
#
#
# NOTE to prevent rebuilds of all files depending on the devicetree header, first compile
# 	   a temporary one and only update the actual header if the content changed
# NOTE Due to the temporary file, make will attempt to rebuild the actual devicetree header
# 	   on each of the build system stages if the devicetree script change without causing a
# 	   change in the devicetree header. To prevent executing the recipe multiple times,
# 	   check for the build stage, only running it on stage1.
define compile_devicetree
	$(if $(call is_prestage,stage1), \
		$(QBUILD)$(dtc) --format=$(1) --sections=$(2) -o $@.tmp $< && \
		{ diff $@ $@.tmp 1>/dev/null 2>&1 || { echo "updating $@"; cp $@.tmp $@; } } \
		, \
	)
endef

$(devtree_preproc): cppflags += -Iscripts/dts/include
$(devtree_preproc): $(CONFIG_DEVICE_TREE) $(config_header)
	$(call preproc_file)

$(devtree_src): $(devtree_preproc) $(dtc)
	$(call compile_devicetree,c,all)

$(devtree_make): $(devtree_preproc) $(dtc)
	$(call compile_devicetree,make,memory$(comma)arch)

$(devtree_header): $(devtree_preproc) $(dtc)
	$(call compile_devicetree,header,memory$(comma)arch)

$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/devicetree.o,          $(devtree_src),)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/devicetree.host.o,     $(devtree_src), host)


####
## device tree inclusion
####

-include $(devtree_make)
