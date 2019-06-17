#
# Copyright (C) 2019 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := start.o isr.o init.o atomic.o $(CONFIG_ARM_BOARD)/


obj-nobuiltin-y := libsys.o

libsys-y := atomic_lib.o

# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%_lib.o,    $(loc_src_tree)/%.c,)


####
## linker scripts
####

## memory layout
lds_memory_script := $(build_tree)/scripts/arch/arm/linker/gen_lds_memory
lds_memory_file := $(loc_build_tree)/memory.lds

$(lds_memory_file): $(lds_memory_script)
	$(mkdir) $(dir $(lds_memory_file))
	$(QBUILD)$(lds_memory_script) $(lds_memory_file)

## combined dependencies
kernel_deps: $(lds_memory_file)
init_deps: $(lds_memory_file)


####
## flags
####

## general flags
cflags += \
	-Os

archflags += \
	-mcpu=$(CONFIG_ARM_CORE) \
	-m$(CONFIG_ARM_ISA)

ifeq ($(CONFIG_BUILD_RELEASE),y)
  ldflags += --discard-all
endif

## kernel flags
ldlibs-kernel-arch := \
	-L$(loc_build_tree) \
	-T$(lds_memory_file) \
	-Wl,--section-start=.text=$(CONFIG_ARM_KERNEL_TEXT_BASE) \
	-Wl,--section-start=.data=$(CONFIG_ARM_KERNEL_DATA_BASE) \
	-Wl,-Map=$(build_tree)/$(basename $(kernel_name)).map,--cref

## init application flags
ldlibs-app-arch := \
	-L$(loc_build_tree) \
	-T$(lds_memory_file) \
	-Wl,--section-start=.text=$(CONFIG_ARM_APP_TEXT_BASE) \
	-Wl,--section-start=.data=$(CONFIG_ARM_APP_DATA_BASE) \
	-Wl,-Map=$(build_tree)/init/init.map,--cref


##############################
###   ARM specific rules   ###
##############################

# open a xterm window using the specified command
#
# 	$(call term,<cmd>,<bg-exec>)
define term
  @xterm -fg white -bg black -geometry 70x10+0+0 -fa 'dejavu sans mono' -fs 7 -T CONFIG_AVR_DEBUGGER -e " \
  	$(1) || \
		(echo -e '\n\n\033[35mpress any key to continue\033[0m'; read -n1) \
	" \
	$(2)
endef

####
## debug
####

.PHONY: debug
debug:
	$(call term, openocd -f $(CONFIG_ARM_OOCD_SCRIPTS)/interface/$(CONFIG_ARM_OOCD_INTERFACE) -f $(CONFIG_ARM_OOCD_SCRIPTS)/board/$(CONFIG_ARM_OOCD_BOARD) -c 'init; reset halt;', &)
	sleep 3; telnet localhost 4444

####
## update flash
####

.PHONY: flash
flash: all
	openocd \
		-f $(CONFIG_ARM_OOCD_SCRIPTS)/interface/$(CONFIG_ARM_OOCD_INTERFACE) \
		-f $(CONFIG_ARM_OOCD_SCRIPTS)/board/$(CONFIG_ARM_OOCD_BOARD) \
		-c "init; reset halt; flash write_image erase \"$(kernel)\"; reset run; shutdown"
ifeq ($(CONFIG_ATSAMV71),y)
	openocd \
		-f $(CONFIG_ARM_OOCD_SCRIPTS)/interface/$(CONFIG_ARM_OOCD_INTERFACE) \
		-f $(CONFIG_ARM_OOCD_SCRIPTS)/board/$(CONFIG_ARM_OOCD_BOARD) \
		-c "init; reset halt; atsamv gpnvm set 1; atsamv gpnvm show; reset run; shutdown"
endif
