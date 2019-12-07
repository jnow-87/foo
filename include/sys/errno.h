/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_ERRNO_H
#define SYS_ERRNO_H


/* macros */
#define return_errno(e_code)		{ errno = e_code; return (-e_code); }
#define goto_errno(label, e_code)	{ errno = e_code; goto label; }


/* types */
typedef enum{
	E_OK = 0,	/**< all ok */
	E_INVAL,	/**< invalid argument, e.g. out of range or entry not found */
	E_NOMEM,	/**< out of memory */
	E_LIMIT,	/**< implementation limit reached */
	E_IO,		/**< I/O error */
	E_NOIMP,	/**< function not implemented */
	E_INUSE,	/**< resource in use */
	E_UNAVAIL,	/**< resource is not available */
	E_END,		/**< end of resource reached */
	E_NOSUP,	/**< operation not supported */
	E_CONN,		/**< connection already established */
	E_NOCONN,	/**< no connection */
	E_UNKNOWN,	/**< unknown error */
} errno_t;


/* external variables */
extern errno_t errno;


#endif // SYS_ERRNO_H
