/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_BLOB_H
#define SYS_BLOB_H


/* macros */
#define BLOB(_buf, n)		((blob_t){ .buf = _buf, .len = n })
#define BLOBS(...)			((blob_t []){ __VA_ARGS__ })


/* types */
typedef struct{
	void *buf;
	size_t len;
} blob_t;


#endif // SYS_BLOB_H
