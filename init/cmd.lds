SECTIONS {
	.data : {
		__cmds_start = .;
		*(.commands*)
		__cmds_end = .;
	} > data_app AT>flash_app
}

INSERT AFTER .text;
