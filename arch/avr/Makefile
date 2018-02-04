obj-y := start.o isr.o interrupt.o core.o timer.o thread.o syscall.o atomic.o
obj-nobuiltin-y := libsys.o
hostobj-y := avrconfig/


libsys-y := core_lib.o lib.o syscall_lib.o atomic.o


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%_lib.o,    $(loc_src_tree)/%.c,)


####
## linker script: memory layout
####

avr_mem_lds_create := scripts/linker/avr_mem_lds_create.sh
avr_mem_lds := $(loc_build_tree)/avr_mem.lds

kernel: gen_avr_mem_linker_script
init: gen_avr_mem_linker_script

gen_avr_mem_linker_script: $(avr_mem_lds)

$(avr_mem_lds): check_configheader $(avr_mem_lds_create)
	$(QBUILD)$(avr_mem_lds_create) $(config) $(avr_mem_lds)


####
## AVR config header
####

avrconfig := $(loc_build_tree)/avrconfig/avrconfig
avrconfig_header := $(dir $(config_header))avrconfig.h

kernel: $(avrconfig_header)

$(avrconfig_header): check_configheader $(avrconfig)
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
ldflags += -m$(CONFIG_AVR_ISA) --discard-all

## kernel flags
ldlibs-kernel-arch := \
	-T$(avr_mem_lds) \
	-Wl,--section-start=.text=$(CONFIG_KERNEL_TEXT_BASE) \
	-Wl,--section-start=.data=$(CONFIG_KERNEL_DATA_BASE) \
	-Wl,-Map=$(build_tree)/$(basename $(kernel_name)).map,--cref

## init application flags
ldlibs-app-arch := \
	-T$(avr_mem_lds) \
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
