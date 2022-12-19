/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86EMU_CHILD_H
#define X86EMU_CHILD_H


#include <sys/types.h>
#include <pthread.h>


/* types */
typedef struct child_pipe_t{
	int tgt_fileno[2];

	int pipe_rd[2],
		pipe_wr[2];

	int fd_rd,
		fd_wr;
} child_pipe_t;

typedef struct{
	char const *name;
	pid_t pid;

	child_pipe_t *pipes;
	size_t npipes;

	pthread_mutex_t mtx;
} child_t;


/* prototypes */
child_t *child_create(char const *name);
void child_destroy(child_t *child);

void child_add_pipe(child_t *child, int fileno_rd, int fileno_wr);
void child_fork(child_t *child, char **argv);

void child_lock(child_t *child);
void child_unlock(child_t *child);

void child_signal(child_t *child, int sig);

void child_read(child_t *child, size_t pipe, void *buf, ssize_t n);
void child_write(child_t *child, size_t pipe, void *buf, ssize_t n);
void child_fwd(child_t *tgt, child_t *src, size_t pipe, ssize_t n);


#endif // X86EMU_CHILD_H
