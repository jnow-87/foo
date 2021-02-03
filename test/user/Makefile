#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := interactive.o non-interactive.o
hostobj-y := $(obj-y)

interactive-y := tty.o gpioint.o
non-interactive-y := close_sync.o thread.o stat.o signal.o kill.o exit.o


interactive-cppflags := -DTEST_TYPE=\"user_interactive\"
interactive-hostcppflags := $(interactive-cppflags)

non-interactive-cppflags := -DTEST_TYPE=\"user_noninteractive\"
non-interactive-hostcppflags := $(non-interactive-cppflags)
