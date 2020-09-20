"
" Copyright (C) 2016 Jan Nowotsch
" Author Jan Nowotsch	<jan.nowotsch@gmail.com>
"
" Released under the terms of the GNU GPL v2.0
"



" get the architecture of kernel_image
function! s:get_arch(kernel_image)
	let l:machine_id = {
		\ "avr":["AVR"],
	\ }

	let l:machine = system("readelf -h " . a:kernel_image . " | grep 'Machine' | cut -d ':' -f 2")

	for l:id in keys(l:machine_id)
		for l:m in l:machine_id[l:id]
			if stridx(l:machine, l:m) != -1
				return l:id
			endif
		endfor
	endfor

	return ""
endfunction


" get kernel image and related architecture
if argc == 0
	echoerr "no kernel image provided"
	finish
endif

let s:kimg = argv[0]
let s:arch = s:get_arch(s:kimg)

if s:arch == ""
	echoerr "unknown architecture"
	finish
endif

" source architecture dependent init
exec "silent! source .vimgdb/" . s:arch . ".vim"

" init vimgdb
let vimgdb_gdblog_show = 0
let vimgdb_callstack_show = 0

Vimgdb start

exec "Inferior " . s:kimg

" open peripherals window
rightbelow 60vsplit
Window view peripherals
silent source .vimgdb/per.vim

" add memory dumps
Window focus source
leftabove 40vsplit
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
