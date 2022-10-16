#
# Copyright (C) 2019 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := dtc


dtc-y := main.o options.o export.o lexer.lex.o parser.tab.o

# ensure a rebuild if indirect dependencies are updated
dtc-y += $(build_tree)/sys/libhost.a

# additional dependencies for generated headers
$(loc_build_tree)/parser.tab.host.o: $(loc_build_tree)/lexer.lex.host.o
$(loc_build_tree)/lexer.lex.host.o: $(loc_build_tree)/parser.tab.c
$(loc_build_tree)/main.host.o: $(loc_build_tree)/parser.tab.c


hostcppflags-y := \
	-I$(src_tree) \
	-I$(src_tree)/include \
	-I$(loc_src_tree) \
	-I$(loc_build_tree) \
	-I/usr/include

hostcflags-y := \
	-fno-builtin

hostldlibs-y := \
	-L$(build_tree)/sys -lhost
