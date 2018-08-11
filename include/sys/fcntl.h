#ifndef SYS_FCNTL_H
#define SYS_FCNTL_H


/* types */
typedef enum{
	F_SEEK = 1,		// implementation required on lowest callback level
	F_TELL,			// implementation required on lowest callback level
	F_MODE_GET,		// implemented at highest callback level
	F_MODE_SET,		// implemented at highest callback level
	F_SYNC,			// implemented at highest callback level
} fcntl_cmd_t;

typedef enum{
	SEEK_SET = 1,
	SEEK_CUR,
	SEEK_END,
} whence_t;

typedef struct{
	whence_t whence;
	size_t pos;
	int offset;
} seek_t;


#endif // SYS_FCNTL_H
