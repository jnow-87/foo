/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H


#include <config/config.h>
#include <kernel/thread.h>
#include <sys/types.h>
#include <sys/syscall.h>


/* macros */
#ifndef CONFIG_KERNEL_SYSCALL
# define sc_khdlr(num, param, psize)	0
#endif // CONFIG_KERNEL_SYSCALL


/* types */
/**
 * \brief	syscall handler callback
 *
 * \param	param	kernel space pointer to the system call parameter
 *
 * \post	errno is set accordingly if an error occurred
 *
 * \return	0	on success
 * 			<0	on error
 */
typedef int (*sc_hdlr_t)(void *param);


/* prototypes */
int sc_register(sc_num_t num, sc_hdlr_t hdlr);
int sc_release(sc_num_t num);

#ifdef CONFIG_KERNEL_SYSCALL
void sc_khdlr(sc_num_t num, void *param, size_t psize);
#endif // CONFIG_KERNEL_SYSCALL


#endif // KERNEL_SYSCALL_H
