#ifndef KERNEL_BINLOADER_H
#define KERNEL_BINLOADER_H


#include <kernel/process.h>
#include <sys/binloader.h>
#include <sys/errno.h>


/* incomplete types */
struct process_t;


/* prototypes */
int bin_load(void *binary, bin_type_t bin_type, struct process_t *this_p, void **entry);


#endif // KERNEL_BINLOADER_H
