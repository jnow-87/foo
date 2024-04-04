"
" Copyright (C) 2016 Jan Nowotsch
" Author Jan Nowotsch	<jan.nowotsch@gmail.com>
"
" Released under the terms of the GNU GPL v2.0
"



function! s:get_config_value(var)
	return trim(system("cat .config | grep -w '^" . a:var . "' | cut -d '=' -f 2"))
endfunction

function! s:source_script(script)
	let l:script = ".vimgdb/" . a:script . ".vim"

	if filereadable(l:script)
		echom "source startup script: " . l:script
		exec "source " . l:script
	endif
endfunction

function! s:open_window(name, where, relative, size, split)
	exec "Window focus " . a:relative
	exec a:where . " " . a:size . a:split . "split"
	exec "Window view " . a:name

	call s:source_script(a:name)
endfunction

function! s:load_per(processor)
	let l:file = ".vimgdb/per/" . a:processor . ".per"

	if filereadable(l:file)
		echom "load peripheral file " . l:file
		exec "Per " . l:file
	endif
endfunction

function! s:verify_arch(image, arch)
	let l:machine_id = {
		\ "arm": ["ARM"],
		\ "avr": ["AVR"],
		\ "x86": ["x86", "X86" ],
	\ }

	let l:machine = trim(system("readelf -h " . a:image . " | grep 'Machine' | cut -d ':' -f 2"))

	for l:id in keys(l:machine_id)
		for l:m in l:machine_id[l:id]
			if stridx(l:machine, l:m) != -1 && l:id == a:arch
				return 0
			endif
		endfor
	endfor

	return -1
endfunction


" verify arguments
if argc == 0
	echoerr "no binary provided"
	finish
endif

let s:image = argv[0]
let s:arch = s:get_config_value("CONFIG_ARCH")
let s:processor = s:get_config_value("CONFIG_PROCESSOR")

if s:verify_arch(s:image, s:arch) != 0
	echoerr "image " . s:image . " doesn't match CONFIG_ARCH=" . s:arch
	finish
endif

" source target-dependent scripts
call s:source_script(s:arch)
call s:source_script(s:processor)

" init vimgdb
let vimgdb_gdblog_show = 0
let vimgdb_callstack_show = 0

Vimgdb start

exec "Inferior " . s:image

" open windows
call s:open_window("peripherals", "rightbelow", "source", 60, "v")
call s:open_window("memory", "leftabove", "source", 40, "v")
call s:open_window("breakpoints", "rightbelow", "peripherals", 10, "")
call s:open_window("callstack", "rightbelow", "source", 15, "")
call s:open_window("variables", "rightbelow", "memory", 10, "")

Window focus source

call s:load_per(s:processor)
call s:source_script("user")
