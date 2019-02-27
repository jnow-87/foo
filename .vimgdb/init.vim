"
" Copyright (C) 2016 Jan Nowotsch
" Author Jan Nowotsch	<jan.nowotsch@gmail.com>
"
" Released under the terms of the GNU GPL v2.0
"


""""
"" check arguments
""""
if argc < 2
	echoerr "usage: ". expand('<sfile>:t') . " <arch> <image>"
	finish
endif

let arch = argv[0]
let kimg = argv[1]

""""
"" check target architecture
""""
if arch == "avr"
	silent !make debug
	let vimgdb_gdb_cmd = 'avr-gdb -ex \"target remote 127.0.0.1:1212\"'

elseif arch == "arm"
	let vimgdb_gdb_cmd = 'arm-none-eabi-gdb -ex \"target remote | openocd  -f /usr/share/openocd/scripts/interface/cmsis-dap.cfg -f /usr/share/openocd/scripts/board/atmel_samv71_xplained_ultra.cfg -c \\\"gdb_port pipe; init; reset halt;\\\"\"'

else
	echoerr "unknown architecture"
	finish
endif

""""
"" init vimgdb
""""
let vimgdb_gdblog_show = 0
let vimgdb_callstack_show = 0

Vimgdb start

exec "Inferior " . kimg

""""
"" init windows
""""
" open peripherals window
rightbelow 80vsplit
Window view peripherals
silent source .vimgdb/per.vim

" add memory dumps
Window focus source
leftabove 80vsplit
Window view memory
silent source .vimgdb/mem.vim

" open breakpoint window
Window focus peripherals
rightbelow 10split
Window view breakpoints
Window focus source
silent source .vimgdb/break.vim

" open callstack window
Window focus source
rightbelow 15split
Window view callstack

" open variables window
Window focus memory
rightbelow 10split
Window view variables
silent source .vimgdb/var.vim

" focus source window
Window focus source
