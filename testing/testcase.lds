SECTIONS {
	.testing : {
		__test_cases_base = .;

		*(.test_cases)

		__test_cases_end = .;
	}
}
