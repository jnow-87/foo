ENTRY(__start)


/** MEMORY sections are defined in a separate script depending on the
 *  configured target MCU, cf. scripts/linker/mem_*
 */

SECTIONS {
	/** dummy section, allowing to define the starting address
	 *  through the command line
	 */
	.base 0x0 : {
	}


	/* interrupt vectors */
	.intvec ADDR(.base) : {
		*(.intvec)

		. += 0x10;
	} > kernel


	/* code */
	.text : {
		*(.text .stub .text.* .gnu.linkonce.t.*)
		*(.gnu.warning)
	} > kernel


	/* data */
	INCLUDE kernel_init.lds

	.data : {
		*(.data .data.* .gnu.linkonce.d.*)
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)
	} > data AT>kernel

	__data_start = ADDR(.data);
	__data_end = __data_start + SIZEOF(.data);
	__data_load_start = LOADADDR(.data);
	__data_load_end = __data_load_start + SIZEOF(.data);


	/* bss */
	.bss : {
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(.dynsbss)
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
		*(.dynsbss)
		*(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
		*(.scommon)
		*(COMMON)
	} > data AT>data

	__bss_start = ADDR(.bss);
	__bss_end = __bss_start + SIZEOF(.bss);


	/* discard sections */
	/DISCARD/ : {
		*(.init4)	/* libgcc .data initialisation not needed */
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
