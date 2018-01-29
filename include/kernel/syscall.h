#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H


#include <kernel/thread.h>
#include <sys/types.h>
#include <sys/syscall.h>


/* types */
/**
 * \brief	syscall handler callback
 *
 * \param	param	kernel space pointer to the system call parameter
 *
 * \post	errno is set accordingly if an error occurred
 *
 * \return	E_OK	on success
 * 			<0		on error
 */
typedef int (*sc_hdlr_t)(void *param);


/* prototypes */
int sc_register(sc_t num, sc_hdlr_t hdlr);
int sc_release(sc_t num);

int ksc_hdlr(sc_t num, void *param, size_t psize);


#endif // KERNEL_SYSCALL_H
