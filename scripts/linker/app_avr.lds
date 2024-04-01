/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devicetree.h>


/* MEMORY sections are defined in a script generated based on the device tree */

ENTRY(avr_start)

SECTIONS {
	/* code */
	.text : {
		*(.avr_start)
		*( .progmem.gcc*)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		*(.gnu.warning)
		*(.trampolines)
	} > app_flash

	/* data */
	.data : {
		*(.data .data.* .gnu.linkonce.d.*)
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)

		INCLUDE std_init.i.lds
		INCLUDE libsys_init.i.lds
		INCLUDE fonts.i.lds

#ifdef BUILD_USER_PLUGIN
		. = ALIGN(DEVTREE_ARCH_ADDR_WIDTH / 8);
		INCLUDE plugin.lds
#endif
	} > app_data AT>app_flash

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
	} > app_data

	__bss_start = ADDR(.bss);
	__bss_end = __bss_start + SIZEOF(.bss);

	/* plugin */
	INCLUDE avr_plugin_app.lds

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
