/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



SECTIONS {
	.data : {
		/* array of commands */
		__cmds_start = .;
		*(.commands*)
		__cmds_end = .;

		/* array of tests */
		__start_tests_user_interactive = .;
		*(tests_user_interactive)
		__stop_tests_user_interactive = .;

		__start_tests_user_noninteractive = .;
		*(tests_user_noninteractive)
		__stop_tests_user_noninteractive = .;
	} > app_data
}

INSERT AFTER .text;
