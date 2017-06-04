#ifndef SYS_ERRNO_H
#define SYS_ERRNO_H


/* macros */
#define return_errno(e)				return (errno = e)
#define goto_errno(label, e_code)	{ errno = e_code; goto label; }


/* types */
typedef enum{
	E_OK = 0,		/**< all ok */
	E_INVAL = -1,	/**< invalid argument, e.g. out of range or entry not found */
	E_NOMEM = -2,	/**< out of memory */
	E_LIMIT = -3,	/**< implementation limit reached */
	E_IO = -4,		/**< I/O error */
	E_NOIMP = -5,	/**< function not implemented */
	E_INUSE = -6,	/**< resource in use */
} errno_t;


/* external variables */
extern errno_t errno;


#endif // SYS_ERRNO_H
