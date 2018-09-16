bin-y := init.elf
hostbin-y := init.host


init-y := main.o $(build_tree)/lib/$(lib_name) cmd/
init.host-y := $(init-y)

$(loc_build_tree)/main.o: $(loc_build_tree)/cmd/cmd.hash.h
$(loc_build_tree)/main.host.o: $(loc_build_tree)/cmd/cmd.hash.h

# additional init linker options
init-hostcppflags := -I$(loc_src_tree) -I$(loc_build_tree)

init-cppflags := -I"include/lib" -I$(loc_src_tree) -I$(loc_build_tree)
init-ldlibs := \
	$(ldlibs-app-arch) \
	-lsys -lgcc

# init binary name required by main Makefile
init := $(loc_build_tree)/init.elf
