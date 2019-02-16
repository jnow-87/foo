#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#


obj-y := devicetree.o

obj-$(CONFIG_DRIVER_LOOPBACK) += loop/
obj-$(CONFIG_DRIVER_TERM) += term/
obj-$(CONFIG_DRIVER_UART) += uart/
obj-$(CONFIG_DRIVER_GPIO) += gpio/
obj-$(CONFIG_DRIVER_PWM) += pwm/


####
## device tree
####

devtree := $(build_tree)/scripts/devtree/devtree

$(loc_build_tree)/devicetree.c: $(CONFIG_DEVICE_TREE) $(devtree)
	$(mkdir) $(dir $@)
	$(QBUILD)$(devtree) $< $@

$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/devicetree.o,    $(loc_build_tree)/devicetree.c,)
