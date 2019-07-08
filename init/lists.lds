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
		__tests_start = .;
		*(.tests*)
		__tests_end = .;
	} > app_data AT>app_flash
}

INSERT AFTER .text;
