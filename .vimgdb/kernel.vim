if argc == 0
	echoerr "no kernel image provided"
	finish
endif

let kimg = argv[0]

silent !make debug

let vimgdb_gdb_cmd = 'avr-gdb -ex \"target remote 127.0.0.1:1212\"'
let vimgdb_gdblog_show = 0

Vimgdb start

exec "Inferior bin " . kimg
exec "Inferior sym " . kimg

Per .vimgdb/per/atmega1284p.per
