obj-y := start.o atmega_kernel.o
obj-nobuiltin-y := libsys.o


%/kimg.elf: ldflags += -Tmem_$(shell echo $(CONFIG_MCU) | grep -o -e '^[a-z]*[0-9]*').lds


libsys-y := atmega_lib.o


$(loc_build_tree)/atmega_kernel.o: $(loc_src_tree)/atmega.c
	$(mkdir) $(dir $@)
	$(call compile_o_c)

$(loc_build_tree)/atmega_lib.o: $(loc_src_tree)/atmega.c
	$(mkdir) $(dir $@)
	$(call compile_o_c)
