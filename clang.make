#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# update .clang file
.PHONY: dotclang
dotclang:
	$(call cmd_run_script,$(echo) $(cppflags) $(hostcppflags) | grep -o -e "-I[ \t]*[a-zA-Z0-9_/\.]*" > .clang || true)
