#ifndef SYS_DIRENT_H
#define SYS_DIRENT_H


#include <config/config.h>
#include <sys/stat.h>


/* types */
typedef struct{
	file_type_t type;
	char name[CONFIG_FILE_NAME_LEN];
} dir_ent_t;


#endif // SYS_DIRENT_H
