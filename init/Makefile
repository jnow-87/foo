bin-y := init.elf


init-y := main.o $(build_tree)/lib/$(lib_name) cmd/

# additional init linker options
init-cppflags := -I"include/lib" -I$(loc_src_tree) -I$(loc_build_tree)
init-ldlibs := \
	$(ldlibs-app-arch) \
	-T$(loc_src_tree)/cmd.lds \
	-lsys \
	-lgcc

# init binary name required by main Makefile
init := $(loc_build_tree)/init.elf
