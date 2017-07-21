#include <kernel/process.h>
#include <kernel/binloader.h>
#include <sys/errno.h>


/* global functions */
int bin_load_raw(void *binary, process_t *this_p, void **entry){
	*entry = binary;

	return_errno(E_OK);
}
