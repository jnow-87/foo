#ifndef SYS_ERRNO_H
#define SYS_ERRNO_H


/* macros */
#define return_errno(e_code)		{ errno |= e_code; return (-e_code); }
#define goto_errno(label, e_code)	{ errno |= e_code; goto label; }


/* types */
typedef enum{
	E_OK = 0,			/**< all ok */
	E_INVAL = 0x1,		/**< invalid argument, e.g. out of range or entry not found */
	E_NOMEM = 0x2,		/**< out of memory */
	E_LIMIT = 0x4,		/**< implementation limit reached */
	E_IO = 0x8,			/**< I/O error */
	E_NOIMP = 0x10,		/**< function not implemented */
	E_INUSE = 0x20,		/**< resource in use */
	E_UNAVAIL = 0x40,	/**< resource is not available */
	E_END = 0x80,		/**< end of resource reached */
} errno_t;


/* external variables */
extern int errno;


#endif // SYS_ERRNO_H
