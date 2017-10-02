#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H


/* macros */
#define kernel_panic(format, ...) \
	_kernel_panic(__FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)


/* prototypes */
void _kernel_panic(char const *file, char const *func, unsigned int line, char const *format, ...);


#endif // KERNEL_PANIC_H
