hostbin-y := main


main-y := main.o string.o vfprintf.o list.o queue.o stack.o memblock.o ringbuf.o sys/

main-hostcppflags += -I$(src_tree) -I$(src_tree)/include -I/usr/include
main-hostcflags += -fno-builtin


# testing rules
.PHONY: test
test: check_config $(loc_build_tree)/main
	$(loc_build_tree)/main $(loc_build_tree)/result.log
