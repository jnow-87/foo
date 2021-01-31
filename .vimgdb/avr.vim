"
" Copyright (C) 2020 Jan Nowotsch
" Author Jan Nowotsch	<jan.nowotsch@gmail.com>
"
" Released under the terms of the GNU GPL v2.0
"



silent !make debug

let vimgdb_gdb_cmd = 'avr-gdb -ex \"target remote 127.0.0.1:1212\"'
