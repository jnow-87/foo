SECTIONS {
	/* code */
	.text : {
		*(.text .stub .text.* .gnu.linkonce.t.*)
		*(.gnu.warning)
	}

	/* data */
	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)
	}

	.data : {
		*(.data .data.* .gnu.linkonce.d.*)
		SORT(CONSTRUCTORS)

		INCLUDE libsys_init.lds
	}

	.bss : {
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
	}

	.sbss : {
		*(.dynsbss)
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
	}

	.sbss2 : {
		*(.dynsbss)
		*(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
		*(.scommon)
	}

	.eh_frame : {
		KEEP(*(.eh_frame))
	}

	/* debugging (DWARF) */
	/* DWARF 1 */
	.debug	: { *(.debug) }
	.line	: { *(.line) }

	/* GNU DWARF 1 extensions */
	.debug_srcinfo	: { *(.debug_srcinfo) }
	.debug_sfnames	: { *(.debug_sfnames) }

	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges	: { *(.debug_aranges) }
	.debug_pubnames	: { *(.debug_pubnames) }

	/* DWARF 2 */
	.debug_info		: { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev	: { *(.debug_abbrev) }
	.debug_line		: { *(.debug_line) }
	.debug_frame	: { *(.debug_frame) }
	.debug_str		: { *(.debug_str) }
	.debug_loc		: { *(.debug_loc) }
	.debug_macinfo	: { *(.debug_macinfo) }
}
