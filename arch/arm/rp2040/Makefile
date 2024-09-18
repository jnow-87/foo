#
# Copyright (C) 2023 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



####
## build
####

# kernel
obj-y := \
	atomic.o \
	bootloader.o \
	clocks.o \
	core.o \
	pads.o \
	resets.o \
	soc.o

obj-$(DEVTREE_ARCH_MULTI_CORE) += multi-core.o

# libbrick
obj-nobuiltin-y := libbrick.o

libbrick-y := \
	atomic.libbrick.o 


# custom rules to generate separate objects files for src files that are required by kernel and libbrick
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.libbrick.o,    $(loc_src_tree)/%.c,)


####
## flash and debug
####

OPENOCD_SPEED := 30000
OPENOCD_SCRIPT_FLASH = \
	adapter speed $(OPENOCD_SPEED); \
	program $1 verify reset exit

OPENOCD_SCRIPT_DEBUG = \
	gdb_port pipe; \
	adapter speed $(OPENOCD_SPEED); \
	init; \
	reset halt;

OPENOCD_SCRIPT_RESET = \
	adapter speed $(OPENOCD_SPEED); \
	init; \
	reset run; \
	shutdown; 

OPENOCD_LOG_DIR := $(loc_build_tree)
OPENOCD_LOG = $(OPENOCD_LOG_DIR)/openocd.$@.log
OPENCD_RAW = \
	openocd \
		-f interface/cmsis-dap.cfg \
		-f target/rp2040.cfg \
		-c '$1' \

OPENCD = \
	$(echo) "[OPENCD] $@ (log:" $(call fg,violet,$(OPENOCD_LOG))")"; \
	$(call OPENCD_RAW,$1) > $(OPENOCD_LOG) 2>&1 \


.PHONY: flash
flash: flash-kernel flash-init

.PHONY: flash-kernel
flash-kernel: all
	$(call cmd_run_script, $(call OPENCD,$(call OPENOCD_SCRIPT_FLASH,$(kernel))))

.PHONY: flash-init
flash-init: all
	$(call cmd_run_script, $(call OPENCD,$(call OPENOCD_SCRIPT_FLASH,$(init))))

.PHONY: debug
debug:
	$(call cmd_run_script, \
		gdb-multiarch \
			-ex "target extended-remote | $(call OPENCD_RAW,$(OPENOCD_SCRIPT_DEBUG))" \
	)

.PHONY: reset
reset:
	$(call cmd_run_script, $(call OPENCD,$(OPENOCD_SCRIPT_RESET)))
