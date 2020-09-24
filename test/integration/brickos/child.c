/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#define _GNU_SOURCE

#include <config/config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <user/debug.h>
#include <brickos/child.h>


/* types */
typedef ssize_t (*io_call_t)(int, void const *, size_t);


/* local/static prototypes */
static void nclose(int *fds, size_t n);
static bool fd_valid(int fd);
static int io_wrapper(int fd, void *buf, ssize_t n, io_call_t call);


/* global functions */
child_t *child_create(char const *name){
	child_t *child;
	char *_name;


	child = calloc(1, sizeof(child_t) + strlen(name) + 1);

	if(child == 0x0)
		return 0x0;

	_name = (((char*)child) + sizeof(child_t));
	strcpy(_name, name);

	pthread_mutex_init(&child->mtx, 0);

	child->name = _name;
	child->pid = -1;

	return child;
}

void child_destroy(child_t *child){
	size_t i;
	child_pipe_t *pipe;


	DEBUG("terminating child %s (pid: %d)\n", child->name, child->pid);

	if(child->pid != -1){
		(void)child_signal(child, SIGKILL);
		(void)waitpid(child->pid, 0x0, 0);
	}

	for(i=0; i<child->npipes; i++){
		pipe = child->pipes + i;

		(void)close(pipe->fd_rd);
		(void)close(pipe->fd_wr);
	}

	pthread_mutex_destroy(&child->mtx);

	free(child->pipes);
	free(child);
}

void child_add_pipe(child_t *child, int fileno_rd, int fileno_wr){
	child_pipe_t *p;


	p = realloc(child->pipes, sizeof(child_pipe_t) * (child->npipes + 1));

	if(p == 0x0)
		goto err_0;

	child->pipes = p;
	p = child->pipes + child->npipes;

	if(pipe(p->pipe_rd) < 0)
		goto err_0;

	if(pipe(p->pipe_wr) < 0)
		goto err_1;

	p->tgt_fileno[0] = fileno_rd;
	p->tgt_fileno[1] = fileno_wr;
	p->fd_rd = p->pipe_rd[0];
	p->fd_wr = p->pipe_wr[1];

	child->npipes++;

	return;


err_1:
	close(p->pipe_rd[0]);
	close(p->pipe_rd[1]);

err_0:
	EEXIT("add pipe to %s failed with %s\n", child->name, strerror(errno));
}

void child_fork(child_t *child, char **argv){
	size_t i;
	int r;
	int fork_check_pipe[2];
	child_pipe_t *p;
	sigset_t sigs;


	DEBUG("forking %s \"%s\"\n", child->name, argv[0]);

	/* create fork-check */
	if(pipe(fork_check_pipe) < 0)
		goto err_0;

	// make the writing end of the pipe CLOEXEC to detect failed exec() calls
	if(fcntl(fork_check_pipe[1], F_SETFD, fcntl(fork_check_pipe[1], F_GETFD) | FD_CLOEXEC))
		goto err_1;

	/* fork */
	child->pid = fork();

	switch(child->pid){
	// error
	case -1:
		goto err_1;

	// child
	case 0:
		r = 0;
		errno = 0;

		// ensure communication signals are not blocked
		r |= sigemptyset(&sigs);
		r |= sigaddset(&sigs, CONFIG_TEST_INT_DATA_SIG);
		r |= sigaddset(&sigs, CONFIG_TEST_INT_CTRL_SIG);

		r |= pthread_sigmask(SIG_UNBLOCK, &sigs, 0x0);

		// place pipes to specific fileno
		for(i=0; i<child->npipes; i++){
			p = child->pipes + i;

			if(fd_valid(p->tgt_fileno[0]) || fd_valid(p->tgt_fileno[1])){
				errno = EBADFD;
				ERROR("some child interface file descriptor alreay in used (%d or %d)\n",
					p->tgt_fileno[0],
					p->tgt_fileno[1]
				);

				break;
			}

			r |= (dup2(p->pipe_rd[1], p->tgt_fileno[1]) == -1 ? 1 : 0);
			r |= (dup2(p->pipe_wr[0], p->tgt_fileno[0]) == -1 ? 1 : 0);

			nclose(p->pipe_rd, 2);
			nclose(p->pipe_wr, 2);
		}

		// exec
		r |= close(fork_check_pipe[0]);

		if(r == 0 && errno == 0)
			execvp(argv[0], argv);

		(void)write(fork_check_pipe[1], &errno, sizeof(errno));
		close(fork_check_pipe[1]);
		_exit(-1);

	// parent
	default:
		// check if exec succeeded
		(void)close(fork_check_pipe[1]);

		if(read(fork_check_pipe[0], &errno, sizeof(errno)) > 0)
			goto err_1;
	}

	(void)close(fork_check_pipe[0]);

	for(i=0; i<child->npipes; i++){
		p = child->pipes + i;

		(void)close(p->pipe_rd[1]);
		(void)close(p->pipe_wr[0]);
	}

	DEBUG("%s pid: %u\n", child->name, child->pid);

	return;


err_1:
	nclose(fork_check_pipe, 2);

err_0:
	EEXIT("creating child process \"%s\" failed with \"%s\"\n", child->name, strerror(errno));
}

void child_lock(child_t *child){
	pthread_mutex_lock(&child->mtx);
}

void child_unlock(child_t *child){
	pthread_mutex_unlock(&child->mtx);
}

void child_signal(child_t *child, int sig){
	if(kill(child->pid, sig) != 0)
		EEXIT("send signal %d to %s failed with %s\n", sig, child->name, strerror(errno));
}

void child_read(child_t *child, size_t pipe, void *buf, ssize_t n){
	if(io_wrapper(child->pipes[pipe].fd_rd, buf, n, (io_call_t)read))
		EEXIT("read from %s failed with %s\n", child->name, strerror(errno));
}

void child_write(child_t *child, size_t pipe, void *buf, ssize_t n){
	if(io_wrapper(child->pipes[pipe].fd_wr, buf, n, write))
		EEXIT("write to %s failed with %s\n", child->name, strerror(errno));
}

void child_fwd(child_t *tgt, child_t *src, size_t pipe, ssize_t n){
	char buf[n];


	child_read(src, pipe, buf, n);
	child_write(tgt, pipe, buf, n);
}


/* local functions */
static void nclose(int *fds, size_t n){
	for(; n>0; n--){
		if(fds[n - 1] != -1)
			(void)close(fds[n - 1]);
	}
}

static bool fd_valid(int fd){
	int eno;


	eno = errno;

	if(fcntl(fd, F_GETFD) != 0){
		errno = eno;

		return false;
	}

	return true;
}

static int io_wrapper(int fd, void *buf, ssize_t n, io_call_t call){
	ssize_t r;


	while(n){
		r = call(fd, buf, n);

		if(r < 0){
			switch(errno){
			case EINTR:
				DEBUG("tolerated i/o error %s\n", strerror(errno));
				break;

			default:
				return -1;
			}
		}
		else
			n -= r;
	}

	return 0;
}
