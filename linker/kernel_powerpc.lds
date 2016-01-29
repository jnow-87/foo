OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)
ENTRY(__start)


/** define memory areas
 *		used to enforce length check of kernel
 */
MEMORY {
	kernel	: ORIGIN = 0x00000000, LENGTH = 2M	/* 2M kernel image */
	reset	: ORIGIN = 0xfffff000, LENGTH = 4k	/* 4k boot/reset page */
}

SECTIONS {
	__isr_base = 0x100;			/* base address for ISRs */
	__boot_base = 0xfffff000;	/* base address for boot code */
	__reset_base = 0xfffffffc;	/* reset vector */


	/** boot/reset part
	 */
	.boot __boot_base : {
		__boot = .;
		*(.boot)
	} > reset

	.reset __reset_base : {
		__start = .;
		*(.reset)
	} > reset

	/** kernel image
	 */
	/* ensure first 0x100 bytes are set to zero */
	.empty 0x0 : {
		. = __isr_base;
	} > kernel = 0x0

	/* ISRs */
	.isr __isr_base : {
		*(.isr)
	} > kernel

	/* code */
	.text ALIGN(0x100) : {
		*(.text .stub .text.* .gnu.linkonce.t.*)
		*(.gnu.warning)
	} > kernel

	/* data */
	INCLUDE kernel_init.lds

	.rodata ALIGN(0x4) : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)
	} > kernel

	.data ALIGN(0x4) : {
		*(.data .data.* .gnu.linkonce.d.*)
		SORT(CONSTRUCTORS)
	} > kernel

	.bss ALIGN(0x4) : {
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
	} > kernel

	.sbss ALIGN(0x4) : {
		*(.dynsbss)
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
	} > kernel

	.sbss2 ALIGN(0x4) : {
		*(.dynsbss)
		*(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
		*(.scommon)
	} > kernel

	.eh_frame ALIGN(0x4) : {
		KEEP(*(.eh_frame))
	} > kernel

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
