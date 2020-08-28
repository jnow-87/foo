#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# init verbosity variables
V ?= 0

ifeq ($(V),0)
  QBUILD := @
  QUTIL := @
endif

ifeq ($(call cond_ge,$(V),1),1)
  QBUILD :=
  QUTIL := @
endif

ifeq ($(call cond_ge,$(V),2),1)
  QBUILD :=
  QUTIL :=
endif

# check project type
ifneq ($(project_type),c)
  ifneq ($(project_type),cxx)
    $(error invalid $$(project_type), choose either 'c' or 'cxx')
  endif
endif

# init build system variables
all_flags := cflags cxxflags cppflags asflags ldflags archflags ldlibs hostcflags hostcxxflags hostcppflags hostasflags hostldflags hostarchflags hostldlibs yaccflags lexflags gperfflags

obj_types := obj obj-nobuiltin hostobj hostobj-nobuiltin
lib_types := lib hostlib
bin_types := bin hostbin
all_types := $(obj_types) $(lib_types) $(bin_types)

all_build_tools := cc cxx as ld ar hostcc hostcxx hostas hostld hostar lex yacc gperf

ifneq ("$(use_coverage_sys)","")
  all_build_tools += gcov
endif

all_user_tools := $(tool_deps)

# init global flag variables
$(foreach flag,$(all_flags), \
    $(eval $(flag) := ) \
)

# init global variables for list of objects, libraries and executables
$(foreach type,$(supported_types), \
	$(eval $(type) :=) \
)

# disable built-in rules
.SUFFIXES:

# disable removal of temporary files
.SECONDARY:

# init source and build tree
$(call set_default,BUILD_TREE, $(default_build_tree))
$(call set_default,SRC_TREE, .)

build_tree := $(patsubst %/,%,$(BUILD_TREE))
src_tree := $(patsubst %/,%,$(SRC_TREE))

# init variables for directory traversal
build := $(scripts_dir)/build.make
included :=

# set default values for compile tools
$(call set_default,CC, gcc)
$(call set_default,CXX, g++)
$(call set_default,AS, as)
$(call set_default,LD, ld)
$(call set_default,AR, ar)

$(call set_default,HOSTCC, gcc)
$(call set_default,HOSTCXX, g++)
$(call set_default,HOSTAS, as)
$(call set_default,HOSTLD, ld)
$(call set_default,HOSTAR, ar)

$(call set_default,LEX, flex)
$(call set_default,YACC, bison)
$(call set_default,GPERF, gperf)

$(call set_default,GCOV, gcov)

# init variables compile tools
cc := $(QBUILD)$(CC)
cxx := $(QBUILD)$(CXX)
as := $(QBUILD)$(AS)
ld := $(QBUILD)$(LD)
ar := $(QBUILD)$(AR)

hostcc := $(QBUILD)$(HOSTCC)
hostcxx := $(QBUILD)$(HOSTCXX)
hostas := $(QBUILD)$(HOSTAS)
hostld := $(QBUILD)$(HOSTLD)
hostar := $(QBUILD)$(HOSTAR)

lex := $(QBUILD)$(LEX)
yacc := $(QBUILD)$(YACC)
gperf := $(QBUILD)$(GPERF)
gperf_c_header := $(QBUILD)$(scripts_dir)/gperf_c_header.sh
gperf_cxx_header := $(QBUILD)$(scripts_dir)/gperf_cxx_header.sh

gcov := $(QBUILD)$(GCOV)

# init shell command wrappers
echo := @echo
printf := @printf
rm := $(QUTIL)rm -rf
mkdir := $(QUTIL)mkdir -p
touch := $(QUTIL)touch
cp := $(QUTIL)cp
mv := $(QUTIL)mv
grep := $(QUTIL)grep
sym_link := $(QUTIL)ln -s
