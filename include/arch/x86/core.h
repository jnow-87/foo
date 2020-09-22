/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_CORE_H
#define X86_CORE_H


#include <arch/x86/thread.h>


/* prototypes */
void x86_core_sleep(void);
void x86_core_panic(thread_ctx_t const *tc);


#endif // X86_CORE_H
