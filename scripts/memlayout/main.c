/* target header */
#include ARCH_HEADER

/* host header */
#include <stdio.h>


/* macros */
// ensure macros for the respective memory areas are available
#ifndef KERNEL_IMG_SIZE
#define KERNEL_IMG_BASE 0x0
#define KERNEL_IMG_SIZE 0x0
#endif

#ifndef KERNEL_STACK_SIZE
#define KERNEL_STACK_BASE 0x0
#define KERNEL_STACK_SIZE 0x0
#endif

#ifndef KERNEL_STACK_CORE_SIZE
#define KERNEL_STACK_CORE_BASE 0x0
#define KERNEL_STACK_CORE_SIZE 0x0
#endif

#ifndef KERNEL_HEAP_SIZE
#define KERNEL_HEAP_BASE 0x0
#define KERNEL_HEAP_SIZE 0x0
#endif

#ifndef PROCESS_SIZE
#define PROCESS_BASE 0x0
#define PROCESS_SIZE 0x0
#endif

#ifndef IO_SIZE
#define IO_BASE 0x0
#define IO_SIZE 0x0
#endif

#ifndef RAMFS_SIZE
#define RAMFS_BASE 0x0
#define RAMFS_SIZE 0x0
#endif

// print table header
#define PRINT_HEAD(name) \
	printf("\n\033[4m\033[47m\033[30m%s:\033[0m\n\033[47m\033[30m         target         base          end        size               \033[0m\n", name);

// print a table line if size is non-zero
#define PRINT_RANGE(name, base, size) \
	if(size) \
		printf("\033[1m%20.20s\033[0m: 0x%8.8x - 0x%8.8x %8.2fk (%#8.8x)\n", name, base, (base) + (size) - 1, (size) / 1024.0, size);

// print an 'unknown' message
#define PRINT_UNKNOWN(name, base, size) \
	printf("\033[1m%20.20s\033[0m: \033[31munknown\033[0m, please define %s and %s\n", name, #base, #size);

// print a table line or unknown message
#define PRINT_RANGE_EE(name, base, size) \
	PRINT_RANGE(name, base, size) \
	else \
		PRINT_UNKNOWN(name, #base, #size);


/* global functions */
int main(){
#if CONFIG_NCORES > 1
	unsigned int i;
#endif


	/* header */
	printf("\n                       \033[4m\033[47m\033[30mbrickos memory layout\033[0m\n");

	/* mandatory */
	PRINT_HEAD("mandatory");

	PRINT_RANGE_EE("kernel image", KERNEL_IMG_BASE, KERNEL_IMG_SIZE);
	PRINT_RANGE_EE("kernel stack", KERNEL_STACK_BASE, KERNEL_STACK_SIZE);

	// stack per core
#if CONFIG_NCORES > 1
	for(i=0; i<CONFIG_NCORES; i++)
#if KERNEL_STACK_CORE_SIZE == 0
		PRINT_UNKNOWN("kernel stack core", "KERNEL_STACK_CORE_BASE", "KERNEL_STACK_CORE_SIZE");
#else
		printf("\033[1m%18.18s%2d\033[0m: 0x%8.8x - 0x%8.8x %8.2fk (%#8.8x)\n", "kernel stack core", i, KERNEL_STACK_CORE_BASE(i), KERNEL_STACK_CORE_BASE(i) + KERNEL_STACK_CORE_SIZE - 1, KERNEL_STACK_CORE_SIZE / 1024.0, KERNEL_STACK_CORE_SIZE);
#endif
#endif

	PRINT_RANGE_EE("kernel heap", KERNEL_HEAP_BASE, KERNEL_HEAP_SIZE);
	PRINT_RANGE_EE("process", PROCESS_BASE, PROCESS_SIZE);

	/* optional */
	PRINT_HEAD("optional");

	PRINT_RANGE("mapped register", IO_BASE, IO_SIZE);
	PRINT_RANGE("filesystem", RAMFS_BASE, RAMFS_SIZE);

#ifdef CONFIG_POWERPC_QORIQ
	PRINT_HEAD("qoriq specific");

	PRINT_RANGE("CCSR", CCSRBAR, CCSRBAR_SIZE);
	PRINT_RANGE("DCSR", DCSRBAR, DCSRBAR_SIZE);
	PRINT_RANGE("DPAA", DPAA_BASE, DPAA_SIZE);
#endif // CONFIG_POWERPC_QORIQ

#ifdef CONFIG_AVR
	PRINT_HEAD("avr specific");

	PRINT_RANGE("sram data", IO_BASE + IO_SIZE, DATA_SIZE);
#endif // CONFIG_AVR

	printf("\n");

	return 0;
}
