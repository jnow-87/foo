/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* MEMORY sections are defined in a script generated based on the device tree */

SECTIONS {
	/* code */
	.text : {
		/* interrupt vectors */
		*(.reset)
		*(.start)
		*(.isr)
		*( .progmem.gcc*)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		*(.gnu.warning)
		*(.trampolines)
	} > kernel_flash

	__kernel_start = ADDR(.text);					/* byte addresses */
	__kernel_end = __kernel_start + SIZEOF(.text);
	__kernel_start_wa = __kernel_start / 2;			/* word addresses */
	__kernel_end_wa = __kernel_end / 2;

	/* data */
	.data : {
		*(.data .data.* .gnu.linkonce.d.*)
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)

		INCLUDE kernel_init.i.lds
		INCLUDE kernel_stat.i.lds
		INCLUDE kernel_test.i.lds
		INCLUDE fonts.i.lds
	} > kernel_data AT>kernel_flash

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
	} > kernel_data

	__bss_start = ADDR(.bss);
	__bss_end = __bss_start + SIZEOF(.bss);

	/* plugin */
	INCLUDE avr_plugin_kernel.lds

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
