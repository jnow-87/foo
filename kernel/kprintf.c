#include <arch/io.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/opt.h>
#include <sys/stream.h>
#include <sys/stdarg.h>



/* static variables */
FILE kout = {
	.buf = 0,
	.pos = 0,
	.state = F_OK,
	.putc = 0x0,
	.puts = 0x0,
};


/* global functions */
void kprintf(kmsg_t lvl, const char* format, ...){
	va_list lst;


	if((kopt.dbg_lvl & lvl) == 0)
		return;

	va_start(lst, format);
	vfprintf(&kout, format, lst);
	va_end(lst);
}


/* local functions */
static error_t kprintf_init(void){
	kout.putc = arch_cbs_kernel.putchar;
	kout.puts = arch_cbs_kernel.puts;

	return E_OK;
}

kernel_init(0, kprintf_init);
