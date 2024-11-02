#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := file.o process.o math.o fs.o error.o memory.o string.o kernel.o
obj-nobuiltin-y := libbrick.o

libbrick-y := file.o process.o math.o fs.o error.o memory.o string.o
libbrick-y := $(patsubst %.o,%.libbrick.o,$(libbrick-y))


# libgcov symbol rename
libsym_rename := scripts/arch/x86/gcov/libsym-rename.sh
gcov_src := $(wildcard $(loc_src_tree)/*.c)

$(loc_build_tree)/libgcov.a: $(libsym_rename) $(gcov_src)
	$(call cmd_run_script, cat $(gcov_src) | $(libsym_rename) $@ $(CONFIG_CC) gcov_)
