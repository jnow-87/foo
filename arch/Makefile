obj-nobuiltin-y := libsys.o

# check if $(arch) is empty to avoid make infinite loop
ifneq ($(CONFIG_ARCH),)
  obj-y += $(CONFIG_ARCH)/
  libsys-y += $(loc_build_tree)/$(CONFIG_ARCH)/libsys.o
endif
