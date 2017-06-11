#include <kernel/process.h>
#include <kernel/binloader.h>
#include <sys/errno.h>


/* types */
typedef errno_t (*binloader)(void *binary, process_t *this_p, void **entry);


/* external prototypes */
errno_t bin_load_raw(void *binary, process_t *this_p, void **entry);


/* static variables */
static const binloader loader_cbs[] = {
	bin_load_raw,	// BIN_RAW, cf. bin_type_t
	0x0,			// BIN_ELF, cf. bin_type_t TODO
};


/* global functions */
errno_t bin_load(void *binary, bin_type_t bin_type, process_t *this_p, void **entry){
	/* check for invalid loader type */
	if(bin_type >= NBINLOADER)
		return E_INVAL;

	/* check if loader is present */
	if(loader_cbs[bin_type] == 0x0)
		return E_INVAL;

	/* call loader */
	return loader_cbs[bin_type](binary, this_p, entry);
}
