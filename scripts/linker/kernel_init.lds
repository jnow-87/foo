/* add .init section containing init routine calls */
.init ALIGN(0x100) : {
	__core_init0_base = .;
	*(.core_init_stage0)

	__core_init1_base = .;
	*(.core_init_stage1)

	__core_init2_base = .;
	*(.core_init_stage2)

	__platform_init0_base = .;
	*(.platform_init_stage0)

	__platform_init1_base = .;
	*(.platform_init_stage1)

	__kernel_init0_base = .;
	*(.kernel_init_stage0)

	__kernel_init1_base = .;
	*(.kernel_init_stage1)

	__kernel_init2_base = .;
	*(.kernel_init_stage2)

	__driver_init0_base = .;
	*(.driver_init_stage0)

	__driver_init1_base = .;
	*(.driver_init_stage1)

	__init_end = .;
} > kernel
