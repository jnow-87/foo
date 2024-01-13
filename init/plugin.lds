/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* array of commands */
__start_cmds = .; 
*(.commands*)
__stop_cmds = .; 

/* array of tests */
__start_tests_unit = .; 
*(tests_unit)
__stop_tests_unit = .; 

__start_tests_user_interactive = .; 
*(tests_user_interactive)
__stop_tests_user_interactive = .; 

__start_tests_user_noninteractive = .; 
*(tests_user_noninteractive)
__stop_tests_user_noninteractive = .; 
