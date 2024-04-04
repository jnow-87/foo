"
" Copyright (C) 2020 Jan Nowotsch
" Author Jan Nowotsch	<jan.nowotsch@gmail.com>
"
" Released under the terms of the GNU GPL v2.0
"



let s:image = argv[0]

let vimgdb_gdb_cmd = 'gdb'
let vimgdb_gdb_cmd .= ' -ex \"handle SIG34 noprint\"'
let vimgdb_gdb_cmd .= ' -ex \"handle SIG35 noprint\"'
let vimgdb_gdb_cmd .= ' -ex \"handle SIG36 noprint\"'
let vimgdb_gdb_cmd .= ' ' . s:image

if argc < 2
	finish
endif

let s:pid = argv[1]

let vimgdb_gdb_cmd .= ' ' . s:pid
