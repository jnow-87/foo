#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := start.o isr.o interrupt.o core.o thread.o syscall.o atomic.o

obj-$(CONFIG_SCHED_PREEMPTIVE) += timer.o
obj-$(CONFIG_KERNEL_TIMER) += timer.o
obj-$(CONFIG_IOVERFLOW_DET) += iovfl.o


obj-nobuiltin-y := libsys.o

libsys-y := core_lib.o lib.o syscall_lib.o atomic.o


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%_lib.o,    $(loc_src_tree)/%.c,)


####
## linker scripts
####

## memory layout
lds_memory_script := $(build_tree)/scripts/arch/avr/linker/gen_lds_memory
lds_memory_file := $(loc_build_tree)/avr_memory.lds

$(lds_memory_file): $(lds_memory_script)
	$(mkdir) $(dir $(lds_memory_file))
	$(QBUILD)$(lds_memory_script) $(lds_memory_file)

## plugin
lds_plugin_script := $(build_tree)/scripts/arch/avr/linker/gen_lds_plugin
lds_plugin_kernel_file := $(loc_build_tree)/avr_plugin_kernel.lds
lds_plugin_app_file := $(loc_build_tree)/avr_plugin_app.lds

$(lds_plugin_kernel_file): $(lds_plugin_script)
	$(mkdir) $(dir $(lds_plugin_kernel_file))
	$(QBUILD)$(lds_plugin_script) $(lds_plugin_kernel_file) kernel

$(lds_plugin_app_file): $(lds_plugin_script)
	$(mkdir) $(dir $(lds_plugin_app_file))
	$(QBUILD)$(lds_plugin_script) $(lds_plugin_app_file) app

## combined dependencies
avr_lds: $(lds_memory_file) $(lds_plugin_kernel_file) $(lds_plugin_app_file)

kernel_deps: avr_lds
init_deps: avr_lds

####
## AVR config header
####

avrconfig := $(build_tree)/scripts/arch/avr/config/avrconfig
avrconfig_header := $(dir $(config_header))avrconfig.h

kernel_deps: $(avrconfig_header)
init_deps: $(avrconfig_header)
libsys_deps: $(avrconfig_header)
prepare_deps: $(avrconfig_header)

$(avrconfig_header): $(avrconfig)
	$(mkdir) $(dir $(avrconfig_header))
	$(QUTIL)$(avrconfig) $(avrconfig_header) $(if $(QBUILD),,-v)


####
## flags
####

## general flags
# disable jump tables if kernel is mapped to addresses larger than 64k
# this is required due to avr-gcc implementation of jump tables
ifeq ($(shell test $$(printf "%u" $(CONFIG_KERNEL_TEXT_BASE)) -gt $$(printf "%u" 0x10000); echo $$?),0)
  cflags-$(CONFIG_AVR) := -fno-jump-tables
endif

archflags += -mmcu=$(CONFIG_AVR_MCU)
ldflags += -m$(CONFIG_AVR_ISA)

ifeq ($(CONFIG_BUILD_RELEASE),y)
  ldflags += --discard-all
endif

## kernel flags
ldlibs-kernel-arch := \
	-L$(loc_build_tree) \
	-T$(lds_memory_file) \
	-Wl,--section-start=.text=$(CONFIG_KERNEL_TEXT_BASE) \
	-Wl,--section-start=.data=$(CONFIG_KERNEL_DATA_BASE) \
	-Wl,-Map=$(build_tree)/$(basename $(kernel_name)).map,--cref

## init application flags
ldlibs-app-arch := \
	-L$(loc_build_tree) \
	-T$(lds_memory_file) \
	-Wl,--section-start=.text=$(CONFIG_APP_TEXT_BASE) \
	-Wl,--section-start=.data=$(CONFIG_APP_DATA_BASE) \
	-Wl,-Map=$(build_tree)/init/init.map,--cref


##############################
###   AVR specific rules   ###
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

# asking for user input, exiting with error if not 'y'
#
#	$(call yesno,<prompt>)
define yesno
  @read -p "$(1) [y/N] " x; \
  test $$x = y || exit 1
endef

####
## debug
####

.PHONY: debug
debug:
ifneq ($(findstring debugwire,$(CONFIG_AVR_PROG_DEBUG_ARGS)),)
	$(call yesno,use debug-wire for debugging?)
	$(call term, \
		echo -e '\033[35menabling debug-wire on the target...\033[0m'; \
		$(CONFIG_AVR_PROG) -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE) -U hfuse:w:0x9f:m; \
		$(CONFIG_AVR_DEBUGGER) -R -P $(CONFIG_AVR_MCU) $(CONFIG_AVR_PROG_DEBUG_ARGS) :1212; \
		echo -e '\n\n\033[35mdisabling debug-wire on the target...\033[0m'; \
		$(CONFIG_AVR_PROG) -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE); \
		$(CONFIG_AVR_PROG) -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE) \
		| grep -C 1000 'Waiting for connection' \
	, &)
else
	$(call term, $(CONFIG_AVR_DEBUGGER) -R -P $(CONFIG_AVR_MCU) $(CONFIG_AVR_PROG_DEBUG_ARGS) :1212 | grep -C 1000 'Waiting for connection', &)
endif

####
## update flash and fuses
####

.PHONY: fuse
fuse:
	$(CONFIG_AVR_FUSER) $(CONFIG_AVR_MCU) $(CONFIG_AVR_PROG_TYPE) $(CONFIG_AVR_PROG_PORT)

.PHONY: flash
flash: all
	$(CONFIG_AVR_PROG) -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE) -U flash:w:$(init)
	sleep 3; $(CONFIG_AVR_PROG) -D -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE) -U flash:w:$(kernel)

####
## read flash
####

objdump := avr-objdump
dump_name := dump.bin

.PHONY: dump-read
dump-read:
	$(CONFIG_AVR_PROG) -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE) -U flash:r:$(dump_name):r
	
.PHONY: dump-disassemble
dump-disassemble: dump-read
	$(objdump) -b binary -m avr -D $(dump_name) | less
