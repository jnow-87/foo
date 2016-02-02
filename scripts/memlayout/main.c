/* janos header */
#include ARCH_HEADER

/* host header */
#include <stdio.h>


int main(){
	unsigned int i;


	printf("       \033[47m\033[30mjanos memory layout as defined in ARCH_MEM\033[0m\n");
	printf("\033[47m\033[30m    target         base          end        size               \033[0m\n");

	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "KIMG", KERNEL_IMG_BASE, KERNEL_IMG_BASE + KERNEL_IMG_SIZE - 1, KERNEL_IMG_SIZE / 1024, KERNEL_IMG_SIZE);
	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "STACK", KERNEL_STACK_BASE, KERNEL_STACK_BASE + KERNEL_STACK_SIZE - 1, KERNEL_STACK_SIZE / 1024, KERNEL_STACK_SIZE);

	for(i=0; i<CONFIG_NCORES; i++)
		printf("%13.13s%2d: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "STACK CORE", i, KERNEL_STACK_CORE_BASE(i), KERNEL_STACK_CORE_BASE(i) + KERNEL_STACK_CORE_SIZE - 1, KERNEL_STACK_CORE_SIZE / 1024, KERNEL_STACK_CORE_SIZE);

	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "HEAP", KERNEL_HEAP_BASE, KERNEL_HEAP_BASE + KERNEL_HEAP_SIZE - 1, KERNEL_HEAP_SIZE / 1024, KERNEL_HEAP_SIZE);
	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "IO", IO_BASE, IO_BASE + IO_SIZE - 1, IO_SIZE / 1024, IO_SIZE);
	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "FS", RAMFS_BASE, RAMFS_BASE + RAMFS_SIZE - 1, RAMFS_SIZE / 1024, RAMFS_SIZE);
	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "PROC", PROCESS_BASE, PROCESS_BASE + PROCESS_SIZE - 1, PROCESS_SIZE / 1024, PROCESS_SIZE);

#ifdef PLATFORM_QORIQ
	printf("\nArchitecture dependent\n");
	printf("\033[47m\033[30m    target         base          end        size               \033[0m\n");
	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "CCSR", CCSRBAR, CCSRBAR + CCSRBAR_SIZE - 1, CCSRBAR_SIZE / 1024, CCSRBAR_SIZE);
	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "DCSR", DCSRBAR, DCSRBAR + DCSRBAR_SIZE - 1, DCSRBAR_SIZE / 1024, DCSRBAR_SIZE);
	printf("%15.15s: 0x%8.8x - 0x%8.8x %8.uk (%#8.8x)\n", "DPAA", DPAA_BASE, DPAA_BASE + DPAA_SIZE - 1, DPAA_SIZE / 1024, DPAA_SIZE);
#endif

	return 0;
}
