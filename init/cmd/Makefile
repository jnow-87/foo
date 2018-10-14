cmds-$(CONFIG_INIT_HELP) += help.o
cmds-$(CONFIG_INIT_CD) += cd.o
cmds-$(CONFIG_INIT_LS) += ls.o
cmds-$(CONFIG_INIT_ECHO) += echo.o
cmds-$(CONFIG_INIT_CAT) += cat.o
cmds-$(CONFIG_INIT_MKDIR) += mkdir.o
cmds-$(CONFIG_INIT_RM) += rm.o
cmds-$(CONFIG_INIT_TEST) += test.o


obj-y := cmd.o $(cmds-y)
hostobj-y := cmd.o $(cmds-y)


$(loc_build_tree)/help.o: $(build_tree)/init/cmd/cmd.hash.h
$(loc_build_tree)/help.host./o: $(build_tree)/init/cmd/cmd.hash.h


cmd-cflags += -Wno-missing-field-initializers
cmd-hostcflags += -Wno-missing-field-initializers
