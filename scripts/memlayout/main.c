/* target header */
#include BUILD_ARCH_HEADER
#include <sys/escape.h>

/* host header */
#include <stdio.h>


/* macros */
#define KERNEL_STACK_CORE_SIZE			(CONFIG_KERNEL_STACK_SIZE / CONFIG_NCORES)
#define KERNEL_STACK_CORE_BASE(core)	(CONFIG_KERNEL_STACK_BASE + ((core) * CONFIG_KERNEL_STACK_CORE_SIZE))

// print table header
#define PRINT_HEAD(name) \
	printf("\n" UNDERLINE BG_WHITE FG_BLACK "%s:" RESET_ATTR "\n" BG_WHITE FG_BLACK "         target         base          end        size               " RESET_ATTR "\n", name);

// print a table line if size is non-zero
#define PRINT_RANGE(name, base, size) \
	if(size) \
		printf(BOLD "%20.20s" RESET_ATTR ": 0x%8.8x - 0x%8.8x %8.2fk (%#8.8x)\n", name, base, (base) + (size) - 1, (size) / 1024.0, size);

// print an 'unknown' message
#define PRINT_UNKNOWN(name, base, size) \
	printf(BOLD "%20.20s" RESET_ATTR ": " FG_RED "unknown" RESET_ATTR ", please define %s and %s\n", name, #base, #size);

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
	printf("\n                       " UNDERLINE BG_WHITE FG_BLACK "brickos memory layout" RESET_ATTR "\n");

	/* mandatory */
	PRINT_HEAD("mandatory");

	PRINT_RANGE_EE("kernel image", CONFIG_KERNEL_IMAGE_BASE, CONFIG_KERNEL_IMAGE_SIZE);
	PRINT_RANGE_EE("kernel stack", CONFIG_KERNEL_STACK_BASE, CONFIG_KERNEL_STACK_SIZE);

	// stack per core
#if CONFIG_NCORES > 1
	for(i=0; i<CONFIG_NCORES; i++)
#if KERNEL_STACK_CORE_SIZE == 0
		PRINT_UNKNOWN("kernel stack core", "KERNEL_STACK_CORE_BASE", "KERNEL_STACK_CORE_SIZE");
#else
		printf(BOLD "%18.18s%2d" RESET_ATTR ": 0x%8.8x - 0x%8.8x %8.2fk (%#8.8x)\n", "kernel stack core", i, KERNEL_STACK_CORE_BASE(i), KERNEL_STACK_CORE_BASE(i) + KERNEL_STACK_CORE_SIZE - 1, KERNEL_STACK_CORE_SIZE / 1024.0, KERNEL_STACK_CORE_SIZE);
#endif
#endif

	PRINT_RANGE_EE("kernel heap", CONFIG_KERNEL_HEAP_BASE, CONFIG_KERNEL_HEAP_SIZE);
	PRINT_RANGE_EE("process heap", CONFIG_KERNEL_PROC_BASE, CONFIG_KERNEL_PROC_SIZE);

	/* optional */
	PRINT_HEAD("optional");

	PRINT_RANGE("mapped register", CONFIG_MREG_BASE, CONFIG_MREG_SIZE);
	PRINT_RANGE("file system", CONFIG_KERNEL_INITRAMFS_BASE, CONFIG_KERNEL_INITRAMFS_SIZE);

#ifdef CONFIG_POWERPC_QORIQ
	PRINT_HEAD("qoriq specific");

	PRINT_RANGE("CCSR", CCSRBAR, CCSRBAR_SIZE);
	PRINT_RANGE("DCSR", DCSRBAR, DCSRBAR_SIZE);
	PRINT_RANGE("DPAA", DPAA_BASE, DPAA_SIZE);
#endif // CONFIG_POWERPC_QORIQ

#ifdef CONFIG_AVR
	PRINT_HEAD("avr specific");

	PRINT_RANGE("kernel flash", CONFIG_KERNEL_TEXT_BASE, CONFIG_KERNEL_TEXT_SIZE);
	PRINT_RANGE("kernel data", CONFIG_KERNEL_DATA_BASE, CONFIG_KERNEL_DATA_SIZE);
	PRINT_RANGE("application flash", CONFIG_APP_TEXT_BASE, CONFIG_APP_TEXT_SIZE);
	PRINT_RANGE("application data", CONFIG_APP_DATA_BASE, CONFIG_APP_DATA_SIZE);
#endif // CONFIG_AVR

	printf("\n");

	return 0;
}
