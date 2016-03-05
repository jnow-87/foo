#ifndef SYS_ERROR_H
#define SYS_ERROR_H


/* types */
typedef enum{
	E_OK = 0,
	E_INVAL = -1,
	E_NOMEM = -2,
	E_NOENT = -3,
	E_IO = -4,
	E_NOIMP = -5,
} error_t;


#endif // SYS_ERROR_H
