#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-y := app.elf


app-y := main.o scripts/linker/app_$(CONFIG_ARCH).lds

# ensure app is rebuild if libs are updated and
# libs exist before build the app
app-y += $(build_tree)/lib/libsys.a


####
# flags
####

app-cppflags += \
	-I"include/lib" \
	-I$(loc_build_tree)

app-ldlibs += \
	$(ldlibs-app-arch) \
	-lsys \
	-lgcc \
	-Lscripts/linker \
	-Tapp_$(CONFIG_ARCH).lds
