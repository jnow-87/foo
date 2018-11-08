/*
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



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
