#include <config/config.h>
#include <stdio.h>


/* types */
typedef struct{
	char const *name;
	unsigned long int base,
					  size;
	unsigned int allow_zero_size;
} region_t;


/* global functions */
int main(void){
	unsigned int i,
				 j;
	const region_t regs[] = {
		{ .name = "kernel image",		.base = CONFIG_KERNEL_IMAGE_BASE,					.size = CONFIG_KERNEL_IMAGE_SIZE,		.allow_zero_size = 0 },
		{ .name = "kernel stack",		.base = CONFIG_KERNEL_STACK_BASE,					.size = CONFIG_KERNEL_STACK_SIZE,		.allow_zero_size = 0 },
		{ .name = "kernel heap",		.base = CONFIG_KERNEL_HEAP_BASE,					.size = CONFIG_KERNEL_HEAP_SIZE,		.allow_zero_size = 0 },
		{ .name = "process heap",		.base = CONFIG_KERNEL_PROC_BASE,					.size = CONFIG_KERNEL_PROC_SIZE,		.allow_zero_size = 0 },
		{ .name = "init ram fs",		.base = CONFIG_KERNEL_INITRAMFS_BASE,				.size = CONFIG_KERNEL_INITRAMFS_SIZE,	.allow_zero_size = 1 },
#ifdef CONFIG_AVR
		{ .name = "kernel data",		.base = (CONFIG_KERNEL_DATA_BASE),		.size = CONFIG_KERNEL_DATA_SIZE,		.allow_zero_size = 0 },
		{ .name = "application data",	.base = (CONFIG_APP_DATA_BASE),		.size = CONFIG_APP_DATA_SIZE,			.allow_zero_size = 0 },
		{ .name = "mapped registers",	.base = (CONFIG_MREG_BASE),			.size = CONFIG_MREG_SIZE,				.allow_zero_size = 0 },
#endif // CONFIG_AVR
	};


	/* check for region availability */
	for(i=0; i<sizeof(regs)/sizeof(region_t); ++i){
		if(regs[i].size == 0 && regs[i].allow_zero_size == 0){
			printf("\terror: %s must not be of zero size, check config\n\n", regs[i].name);
			return 1;
		}
	}

	/* check region overlap */
	for(i=0; i<sizeof(regs)/sizeof(region_t); ++i){
		// skip zero size regions
		if(regs[i].size == 0)
			continue;

		for(j=i+1; j<sizeof(regs)/sizeof(region_t); ++j){
			// skip zero size regions
			if(regs[j].size == 0)
				continue;

			// check overlap of regions i and j
			if(!((regs[i].base > regs[j].base + regs[j].size -1) || (regs[j].base > regs[i].base + regs[i].size - 1))){
				printf("\terror: %s and %s regions overlap, check config\n\n", regs[i].name, regs[j].name);
				printf("\t%20s %10s %10s %10s\n", "region", "base", "end", "size");
				printf("\t%20s %#10x %#10x %10u\n", regs[i].name, regs[i].base, regs[i].base + regs[i].size - 1, regs[i].size);
				printf("\t%20s %#10x %#10x %10u\n\n", regs[j].name, regs[j].base, regs[j].base + regs[j].size - 1, regs[j].size);

				return 1;
			}
		}
	}

	return 0;
}
