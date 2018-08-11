bin-y := init.elf


init-y := main.o $(build_tree)/lib/$(lib_name)

# additional init linker options
init-cppflags := -I"include/lib"
init-ldlibs := \
	$(ldlibs-app-arch) \
	-lsys -lgcc

# init binary name required by main Makefile
init := $(loc_build_tree)/init.elf
