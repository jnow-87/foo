/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* MEMORY sections are defined in a script generated based on the device tree */

ENTRY(_start)

SECTIONS {
	/* code */
	.text : {
		*(.interp)
		*(.note.gnu.build-id)
		*( .progmem.gcc*)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		*(.gnu.warning)
		*(.trampolines)
	} > text

	__executable_start = ADDR(.text);

	/* data */
	.data : {
		*(.data .data.* .gnu.linkonce.d.*)

		INCLUDE std_init.i.lds
		INCLUDE kernel_init.i.lds
		INCLUDE kernel_stat.i.lds
		INCLUDE kernel_test.i.lds
		INCLUDE fonts.i.lds
	} > kernel_data

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)
	} > kernel_data

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
	} > kernel_data

	/* discard sections */
	INCLUDE discard.i.lds

	/* debugging (DWARF) */
	/* DWARF 1 */
	.debug	0 : { *(.debug) }
	.line	0 : { *(.line) }

	/* GNU DWARF 1 extensions */
	.debug_srcinfo	0 : { *(.debug_srcinfo) }
	.debug_sfnames	0 : { *(.debug_sfnames) }

	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_pubnames	0 : { *(.debug_pubnames) }

	/* DWARF 2 */
	.debug_info		0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_line		0 : { *(.debug_line) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_str		0 : { *(.debug_str) }
	.debug_loc		0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }
}
