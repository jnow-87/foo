/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <config/config.h>
#include <stdio.h>


/* types */
typedef struct{
	char const *name;
	unsigned long int base,
					  size,
					  size_max;
	unsigned int allow_zero_size;
} region_t;


/* global functions */
int main(void){
	unsigned int i,
				 j;
	const region_t reg_ovl[] = {
		{ .name = "kernel image",		.base = CONFIG_KERNEL_IMAGE_BASE,		.size = CONFIG_KERNEL_IMAGE_SIZE,		.size_max = -1,		.allow_zero_size = 0 },
		{ .name = "kernel stack",		.base = CONFIG_KERNEL_STACK_BASE,		.size = CONFIG_KERNEL_STACK_SIZE,		.size_max = -1,		.allow_zero_size = 0 },
		{ .name = "kernel heap",		.base = CONFIG_KERNEL_HEAP_BASE,		.size = CONFIG_KERNEL_HEAP_SIZE,		.size_max = -1,		.allow_zero_size = 0 },
		{ .name = "process heap",		.base = CONFIG_KERNEL_PROC_BASE,		.size = CONFIG_KERNEL_PROC_SIZE,		.size_max = -1,		.allow_zero_size = 0 },
		{ .name = "init ram fs",		.base = CONFIG_KERNEL_INITRAMFS_BASE,	.size = CONFIG_KERNEL_INITRAMFS_SIZE,	.size_max = -1,		.allow_zero_size = 1 },
#ifdef CONFIG_AVR
		{ .name = "kernel data",		.base = (CONFIG_KERNEL_DATA_BASE),		.size = CONFIG_KERNEL_DATA_SIZE,		.size_max = -1,		.allow_zero_size = 0 },
		{ .name = "application data",	.base = (CONFIG_APP_DATA_BASE),			.size = CONFIG_APP_DATA_SIZE,			.size_max = -1,		.allow_zero_size = 0 },
		{ .name = "mapped registers",	.base = (CONFIG_MREG_BASE),				.size = CONFIG_MREG_SIZE,				.size_max = -1,		.allow_zero_size = 0 },
#endif // CONFIG_AVR

		// end of array
		{ .name = ""},
	};

	const region_t reg_size[] = {
#ifdef CONFIG_AVR
		{ .name = "flash",		.size_max = FLASH_SIZE,		.size = CONFIG_KERNEL_TEXT_SIZE + CONFIG_APP_TEXT_SIZE },
		{ .name = "sram",		.size_max = SRAM_SIZE,		.size = CONFIG_KERNEL_STACK_SIZE + CONFIG_KERNEL_HEAP_SIZE
																  + CONFIG_KERNEL_PROC_SIZE + CONFIG_MREG_SIZE
																  + CONFIG_KERNEL_INITRAMFS_SIZE + CONFIG_KERNEL_DATA_SIZE
																  + CONFIG_APP_DATA_SIZE },
#endif // CONFIG_AVR

		// end of array
		{ .name = ""},
	};


	/* check for region availability */
	for(i=0; reg_ovl[i].name[0]!=0; ++i){
		if(reg_ovl[i].size == 0 && reg_ovl[i].allow_zero_size == 0){
			printf("\terror: %s must not be of zero size, check config\n\n", reg_ovl[i].name);
			return 1;
		}
	}

	/* check region overlap */
	for(i=0; reg_ovl[i].name[0]!=0; ++i){
		// skip zero size regions
		if(reg_ovl[i].size == 0)
			continue;

		for(j=i+1; j<sizeof(reg_ovl)/sizeof(region_t); ++j){
			// skip zero size regions
			if(reg_ovl[j].size == 0)
				continue;

			// check overlap of regions i and j
			if(!((reg_ovl[i].base > reg_ovl[j].base + reg_ovl[j].size - 1) || (reg_ovl[j].base > reg_ovl[i].base + reg_ovl[i].size - 1))){
				printf("\terror: %s and %s regions overlap, check config\n\n", reg_ovl[i].name, reg_ovl[j].name);
				printf("\t%20s %10s %10s %10s\n", "region", "base", "end", "size");
				printf("\t%20s %#10x %#10x %10u\n", reg_ovl[i].name, reg_ovl[i].base, reg_ovl[i].base + reg_ovl[i].size - 1, reg_ovl[i].size);
				printf("\t%20s %#10x %#10x %10u\n\n", reg_ovl[j].name, reg_ovl[j].base, reg_ovl[j].base + reg_ovl[j].size - 1, reg_ovl[j].size);

				return 1;
			}
		}
	}

	/* check region size */
	for(i=0; reg_size[i].name[0]!=0; ++i){
		if(reg_size[i].size > reg_size[i].size_max){
			printf("\terror: %s uses %u of %u bytes\n\n", reg_size[i].name, reg_size[i].size, reg_size[i].size_max);
			return 1;
		}
	}

	/* check target architecture specifics */
#ifdef CONFIG_AVR
	// check init binary starting address
	if(CONFIG_INIT_BINTYPE_RAW){
		if(CONFIG_INIT_BINARY != CONFIG_APP_TEXT_BASE){
			printf("\terror: (raw) init binary starting address does not match it's flash address\n\t%s (%#x)  != %s (%#x)\n", "CONFIG_INIT_BINARY", CONFIG_INIT_BINARY, "CONFIG_APP_TEXT_BASE", CONFIG_APP_TEXT_BASE);
			return 1;
		}
	}
#endif // CONFIG_AVR

	return 0;
}
