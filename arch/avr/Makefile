obj-y := start.o isr.o interrupt.o core.o uart.o
obj-nobuiltin-y := libsys.o


libsys-y := core.o


# flags
cppflags-$(CONFIG_AVR) := -Os


# AVR specific flags
%/$(kernel_name): ldlibs += \
	-Tmem_$(shell echo $(CONFIG_AVR_MCU) | grep -o -e '^[a-z]*[0-9]*').lds \
	-Wl,-Map=$(basename $@).map,--cref

archflags += -mmcu=$(CONFIG_AVR_MCU)
ldflags += -m$(CONFIG_AVR_ISA) --discard-all


# AVR specific rules

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

# asking for using input, exiting with error if not 'y'
#
#	$(call yesno,<prompt>)
define yesno
  @read -p "$(1) [y/N] " x; \
  test $$x = y || exit 1
endef


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

.PHONY: fuse
fuse:
	$(CONFIG_AVR_FUSER) $(CONFIG_AVR_MCU) $(CONFIG_AVR_PROG_TYPE) $(CONFIG_AVR_PROG_PORT)

.PHONY: erase
erase:
	$(CONFIG_AVR_PROG) -e -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE)

.PHONY: flash
flash: kernel sysroot
	$(CONFIG_AVR_PROG) -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE) -U flash:w:$(kernel)

.PHONY: dump
dump:
	$(CONFIG_AVR_PROG) -p $(CONFIG_AVR_MCU) -P $(CONFIG_AVR_PROG_PORT) -c $(CONFIG_AVR_PROG_TYPE) -U flash:r:dump:r
