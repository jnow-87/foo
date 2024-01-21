/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_ERRNOT_H
#define SYS_ERRNOT_H


typedef enum{
	E_INVAL = 1,	/**< invalid argument, e.g. out of range or entry not found */
	E_NOMEM,		/**< out of memory */
	E_LIMIT,		/**< implementation limit reached */
	E_IO,			/**< I/O error */
	E_NOIMP,		/**< function not implemented */
	E_INUSE,		/**< resource in use */
	E_UNAVAIL,		/**< resource is not available */
	E_AGAIN,		/**< no data available, try again */
	E_END,			/**< end of resource reached */
	E_NOSUP,		/**< operation not supported */
	E_CONN,			/**< connection already established */
	E_NOCONN,		/**< no connection */
	E_UNKNOWN,		/**< unknown error */
} errno_t;


#endif // SYS_ERRNOT_H
