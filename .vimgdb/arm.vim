"
" Copyright (C) 2023 Jan Nowotsch
" Author Jan Nowotsch	<jan.nowotsch@gmail.com>
"
" Released under the terms of the GNU GPL v2.0
"



" openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "                     init; reset halt; flash write_image /home/jan/dev/pi-pico/examples/build/hello_world/serial/hello_serial.elf; reset run; shutdown
" openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 30000; init; reset halt; flash write_image /home/jan/dev/pi-pico/examples/build/hello_world/serial/hello_serial.elf; flash verify_image /home/jan/dev/pi-pico/examples/build/hello_world/serial/hello_serial.elf; reset run; shutdown"


let s:openocd_bin = 'openocd'
let s:openocd_speed = '30000'
let s:openocd_target = 'rp2040'
let s:openocd_itf = 'cmsis-dap'

let s:openocd_script = 'gdb_port pipe;'
let s:openocd_script .= ' adapter speed ' . s:openocd_speed . ';'
" let s:openocd_script .= ' log_output openocd.log;'
let s:openocd_script .= ' init;'
let s:openocd_script .= ' reset halt;'

let s:openocd_cmd = s:openocd_bin
let s:openocd_cmd .= ' -f interface/' . s:openocd_itf . '.cfg'
let s:openocd_cmd .= ' -f target/' . s:openocd_target . '.cfg'
let s:openocd_cmd .= ' -c \\\"' . s:openocd_script . '\\\"'

let vimgdb_gdb_cmd = 'gdb-multiarch'
let vimgdb_gdb_cmd .= ' -ex \"target extended-remote | ' . s:openocd_cmd . '\"'
