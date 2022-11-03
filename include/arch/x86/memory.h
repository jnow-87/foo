/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_MEMORY_H
#define X86_MEMORY_H


/* incomplete types */
struct process_t;


/* prototypes */
int x86_copy_from_user(void *kernel, void const *user, size_t n, struct process_t const *this_p);
int x86_copy_to_user(void *user, void const *kernel, size_t n, struct process_t const *this_p);

void x86_memory_cleanup(void);


#endif // X86_MEMORY_H
