/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_BLOB_H
#define SYS_BLOB_H


/* macros */
#define BLOB(_data, _len)	((blob_t){ .data = _data, .len = _len })
#define BLOBS(...)			((blob_t []){ __VA_ARGS__ })


/* types */
typedef struct{
	void *data;
	size_t len;
} blob_t;


#endif // SYS_BLOB_H
