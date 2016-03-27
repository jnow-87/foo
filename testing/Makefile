hostbin-y := main


main-y := main.o sys/

main-hostcppflags += -I$(src_tree) -I$(src_tree)/include -I/usr/include
main-hostcflags += -fno-builtin
main-hostldlibs += -T$(loc_src_tree)/default.lds -T$(loc_src_tree)/testcase.lds


# testing rules
.PHONY: test
test: check_config check_configheader $(loc_build_tree)/main
	$(loc_build_tree)/main $(loc_build_tree)/result.log
