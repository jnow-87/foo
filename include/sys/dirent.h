#ifndef SYS_DIRENT_H
#define SYS_DIRENT_H


#include <config/config.h>


/* types */
typedef enum{
	FT_BLK = 1,
	FT_CHR,
	FT_DIR,
	FT_REG,
	FT_LNK,
} file_type_t;

typedef struct{
	file_type_t type;
	char name[CONFIG_FILE_NAME_LEN];
} dir_ent_t;


#endif // SYS_DIRENT_H
