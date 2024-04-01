/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_ERRNO_H
#define SYS_ERRNO_H


#include <config/config.h>
#include <sys/errnums.h>

#ifdef BUILD_KERNEL
# include <arch/arch.h>
# include <sys/devicetree.h>
#endif // BUILD_KERNEL


/* macros */
#ifdef BUILD_KERNEL
# define errno		(_errno[PIR])
# define errno_file	(_errno_file[PIR])
# define errno_line	(_errno_line[PIR])
#else
# define errno		_errno
# define errno_file	_errno_file
# define errno_line	_errno_line
#endif // BUILD_KERNEL

#ifdef CONFIG_EXTENDED_ERRNO
# define set_errno(e_code){ \
	errno = e_code; \
	errno_file = __FILE__; \
	errno_line = __LINE__; \
}
#else
# define set_errno(e_code){ \
	errno = e_code; \
}
#endif // CONFIG_EXTENDED_ERRNO

#define reset_errno()				set_errno(0)
#define return_errno(e_code)		{ set_errno(e_code); return (-e_code); }
#define goto_errno(label, e_code)	{ set_errno(e_code); goto label; }


/* external variables */
#ifdef BUILD_KERNEL
extern errno_t _errno[DEVTREE_ARCH_NCORES];

# ifdef CONFIG_EXTENDED_ERRNO
extern char const *_errno_file[DEVTREE_ARCH_NCORES];
extern unsigned int _errno_line[DEVTREE_ARCH_NCORES];
# endif // CONFIG_EXTENDED_ERRNO
#else
extern errno_t _errno;

# ifdef CONFIG_EXTENDED_ERRNO
extern char const *_errno_file;
extern unsigned int _errno_line;
# endif // CONFIG_EXTENDED_ERRNO
#endif // BUILD_KERNEL


#endif // SYS_ERRNO_H
