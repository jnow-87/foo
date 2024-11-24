#
# Copyright (C) 2023 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := \
	$(CONFIG_ARM_FAMILY)/ \
	$(CONFIG_ARM_CHIP)/

# libbrick
obj-nobuiltin-y := libbrick.o

libbrick-y := \
	$(loc_build_tree)/$(CONFIG_ARM_FAMILY)/libbrick.o \
	$(loc_build_tree)/$(CONFIG_ARM_CHIP)/libbrick.o


####
## arm-specific device tree
###

arm_devtree_make := $(loc_build_tree)/devicetree.make

$(arm_devtree_make): $(devtree_preproc) $(dtc)
	$(call compile_devicetree,make,kernel-flash$(comma)kernel-data$(comma)app-flash$(comma)app-data)

prepare_deps: $(arm_devtree_make)

-include $(arm_devtree_make)


####
## flags
####

## general flags
archflags += \
	-mcpu=$(CONFIG_ARM_CORE) \
	-m$(CONFIG_ARM_ISA)

ifeq ($(CONFIG_BUILD_RELEASE),y)
  ldflags += -Wl,--discard-all
endif

## kernel flags
ldlibs-kernel += \
	-L$(loc_build_tree) \
	-Wl,--section-start=.text=$(DEVTREE_KERNEL_FLASH_BASE) \
	-Wl,--section-start=.data=$(DEVTREE_KERNEL_DATA_BASE) \
	-Wl,-Map=$(build_tree)/kernel/kernel.map,--cref

## init application flags
ldlibs-app += \
	-L$(loc_build_tree) \
	-Wl,--section-start=.text=$(DEVTREE_APP_FLASH_BASE) \
	-Wl,--section-start=.data=$(DEVTREE_APP_DATA_BASE) \
	-Wl,-Map=$(build_tree)/init/init.map,--cref
