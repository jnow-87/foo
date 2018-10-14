bin-y := init.elf


init-y := main.o $(build_tree)/lib/$(lib_name) cmd/

$(loc_build_tree)/main.o: $(loc_build_tree)/cmd/cmd.hash.h

# additional init linker options
init-cppflags := -I"include/lib" -I$(loc_src_tree) -I$(loc_build_tree)
init-ldlibs := \
	$(ldlibs-app-arch) \
	-lsys -lgcc

# init binary name required by main Makefile
init := $(loc_build_tree)/init.elf
