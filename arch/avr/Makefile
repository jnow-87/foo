obj-y := start.o atmega_kernel.o
obj-nobuiltin-y := libsys.o


libsys-y := atmega_lib.o


# object versions for kernel and lib
$(loc_build_tree)/atmega_kernel.o: $(loc_src_tree)/atmega.c
	$(mkdir) $(dir $@)
	$(call compile_o_c)

$(loc_build_tree)/atmega_lib.o: $(loc_src_tree)/atmega.c
	$(mkdir) $(dir $@)
	$(call compile_o_c)


# AVR specific flags
%/$(kernel_name): ldlibs += \
	-Tmem_$(shell echo $(CONFIG_AVR_MCU) | grep -o -e '^[a-z]*[0-9]*').lds \
	-Wl,-Map=$(basename $@).map,--cref

archflags += -mmcu=$(CONFIG_AVR_MCU)
ldflags += -m$(CONFIG_AVR_ISA)
