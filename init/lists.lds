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
	} > data_app AT>flash_app
}

INSERT AFTER .text;
