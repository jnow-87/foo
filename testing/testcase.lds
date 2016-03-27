SECTIONS {
	.testing : {
		__test_case_hdlr_base = .;

		*(.test_case_hdlr)

		__test_case_hdlr_end = .;

		__test_case_desc_base = .;

		*(.test_case_desc)

		__test_case_desc_end = .;
	}
}
