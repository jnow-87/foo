if argc == 0
	echoerr "no kernel image provided"
	finish
endif

let kimg = argv[0]

silent !make debug

let vimgdb_gdb_cmd = 'avr-gdb -ex \"target remote 127.0.0.1:1212\"'
let vimgdb_gdblog_show = 0
let vimgdb_callstack_show = 0

Vimgdb start

exec "Inferior " . kimg

" open peripherals window
rightbelow 60vsplit
Window view peripherals
silent! source .vimgdb/per.vim

" add memory dumps
Window focus source
40vsplit
Window view memory
silent! source .vimgdb/mem.vim

" open breakpoint window
Window focus peripherals
rightbelow 10split
Window view breakpoints
Window focus source
silent! source .vimgdb/break.vim

" open callstack window
Window focus source
rightbelow 15split
Window view callstack

" open variables window
Window focus memory
rightbelow 10split
Window view variables
silent! source .vimgdb/var.vim

" focus source window
Window focus source
