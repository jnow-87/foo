/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <arch/x86/hardware.h>
#include <sys/signalfd.h>
#include <sys/compiler.h>
#include <user/debug.h>
#include <user/opts.h>
#include <user/user.h>
#include <user/term.h>
#include <hardware/hardware.h>
#include <brickos/brickos.h>


/* macros */
#define THREAD(_name, _when, _init, _call, _cleanup){ \
	.tid = 0, \
	.name = _name, \
	.when = _when, \
	.init = _init, \
	.call = _call, \
	.cleanup = _cleanup, \
}


/* types */
typedef struct{
	pthread_t tid;
	char const *name;
	app_mode_t when;

	int (*init)(void);
	void (*call)(void);
	void (*cleanup)(void);
} thread_cfg_t;


/* local/static prototypes */
static void *thread_wrapper(void *arg);

static int signal_hdlr(int fd);
static void verify_signals(void);

static void exit_hdlr(int status, void *arg);
static void cleanup(void);

static void process_info(void);


/* static variables */
static int volatile exit_status = 7;

static thread_cfg_t threads[] = {
	THREAD("interrupts",		AM_ALWAYS,			0x0,				hw_int_process,		0x0),
	THREAD("hardware-event",	AM_ALWAYS,			0x0,				hw_event_process,	0x0),
	THREAD("timer",				AM_NONINTERACTIVE,	0x0,				hw_timer,			0x0),
	THREAD("user-input",		AM_INTERACTIVE,		user_input_help,	user_input_process,	user_input_cleanup),
	THREAD("uart",				AM_ALWAYS,			uart_init,			uart_poll,			uart_cleanup),
	THREAD("display",			AM_ALWAYS,			display_init,		display_poll,		display_cleanup),
};


/* global functions */
int main(int argc, char **argv){
	int r = 0;
	int sig_fd;
	sigset_t sig_lst;


	if(opts_parse(argc, argv))
		return 1;

	/* init signal handling */
	verify_signals();

	r |= sigemptyset(&sig_lst);
	r |= sigaddset(&sig_lst, SIGINT);
	r |= sigaddset(&sig_lst, SIGPIPE);
	r |= sigaddset(&sig_lst, SIGCHLD);
	r |= sigaddset(&sig_lst, CONFIG_X86EMU_USR_SIG);
	r |= sigaddset(&sig_lst, CONFIG_X86EMU_UART_SIG);

	for(size_t i=0; i<X86_INT_PRIOS; i++)
		r |= sigaddset(&sig_lst, CONFIG_X86EMU_HW_SIG + i);

	// ensure none of the threads gets any of the above signals
	r |= pthread_sigmask(SIG_BLOCK, &sig_lst, 0x0);

	sig_fd = signalfd(-1, &sig_lst, 0);

	if(r != 0 || sig_fd == -1)
		EEXIT("signal initialisation failed with %s\n", strerror(errno));

	/* terminal settings */
	if(opts.app_mode == AM_NONINTERACTIVE)
		term_noncanon();

	/* create brickos child proceses */
	if(brickos_init_childs() != 0)
		EEXIT("creating child processes failed\n");

	/* create threads */
	for(size_t i=0; i<sizeof_array(threads); i++){
		if((threads[i].when & opts.app_mode) == 0)
			continue;

		DEBUG(1, "create %s thread\n", threads[i].name);

		if(pthread_create(&threads[i].tid, 0, thread_wrapper, threads + i) != 0)
			EEXIT("creating thread %s failed with %s\n", threads[i].name, strerror(errno));
	}

	/* signal handler */
	if(on_exit(exit_hdlr, 0x0) != 0)
		EEXIT("register exit handler failed with %s\n", strerror(errno));

	/* print process info */
	if(opts.info)
		process_info();

	/* main event loop */
	return signal_hdlr(sig_fd);
}


/* local functions */
static void *thread_wrapper(void *arg){
	thread_cfg_t *cfg = (thread_cfg_t*)arg;


	if(cfg->init && cfg->init() != 0)
		EEXIT("%s thread init failed %s\n", cfg->name, strerror(errno));

	while(1){
		cfg->call();
	}

	return 0x0;
}

static int signal_hdlr(int fd){
	struct signalfd_siginfo info;
	child_t *src;


	while(1){
		if(read(fd, &info, sizeof(info)) != sizeof(info))
			EEXIT("reading signal info failed with %s\n", strerror(errno));

		switch(info.ssi_signo){
		case SIGCHLD:
			exit_status = info.ssi_status;
			DEBUG(1, "recv child-exit from %s with status %d\n", brickos_child_name(info.ssi_pid), exit_status);

			if(exit_status != 0)
				ERROR("%s process stopped with status %d\n", brickos_child_name(info.ssi_pid), exit_status);

			// fall through
		case SIGINT:
		case SIGPIPE: // fall through
			DEBUG(1, "initiate shutdown\n");
			cleanup();

			DEBUG(1, "exit with error code %d\n", exit_status);
			_exit(exit_status);
			break;

		case CONFIG_X86EMU_HW_SIG:
			src = ((pid_t)info.ssi_pid == KERNEL->pid) ? KERNEL : APP;

			DEBUG(2, "enqueue hardware event from %s\n", src->name);
			hw_event_enqueue(src);
			break;

		case CONFIG_X86EMU_USR_SIG: // fall through
		default:
			EEXIT("invalid signal\n");
		}
	}
}

static void verify_signals(){
	int rt_sigs[X86_INT_PRIOS + 1];


	STATIC_ASSERT(CONFIG_X86EMU_HW_SIG != CONFIG_X86EMU_USR_SIG);
	STATIC_ASSERT(CONFIG_X86EMU_HW_SIG != CONFIG_X86EMU_USIGNAL_SIG);
	STATIC_ASSERT(CONFIG_X86EMU_USR_SIG != CONFIG_X86EMU_USIGNAL_SIG);

	rt_sigs[X86_INT_PRIOS] = CONFIG_X86EMU_USR_SIG;

	for(size_t i=0; i<X86_INT_PRIOS; i++)
		rt_sigs[i] = CONFIG_X86EMU_HW_SIG + i;

	for(size_t i=0; i<X86_INT_PRIOS + 1; i++){
		if(rt_sigs[i] < SIGRTMIN || rt_sigs[i] > SIGRTMAX)
			EEXIT("%d is not a real-time signal (%d - %d\n", rt_sigs[i], SIGRTMIN, SIGRTMAX);

		for(size_t j=0; j<X86_INT_PRIOS + 1; j++){
			if(i != j && rt_sigs[i] == rt_sigs[j])
				EEXIT("signal %u used multiple times\n", rt_sigs[i])
		}
	}
}

static void exit_hdlr(int status, void *arg){
	exit_status = status;

	DEBUG(1, "trigger program cleanup\n");
	kill(getpid(), SIGINT);
	pthread_exit(0x0);
}

static void cleanup(void){
	for(size_t i=0; i<sizeof_array(threads); i++){
		if((threads[i].when & opts.app_mode) == 0)
			continue;

		DEBUG(1, "terminating %s thread\n", threads[i].name);

		/**
		 * NOTE Do not join threads.
		 * 	Some threads might be blocked in pthread_mutex_lock()
		 * 	which they will never acquire due to other threads
		 * 		- being closed already or
		 * 		- waiting for responses from the brickos processes,
		 * 		  which already terminated
		 *
		 * 	Since pthread_mutex_lock() is no cancelation point, it
		 * 	will block forever and would likewise block pthread_join()
		 */
		pthread_cancel(threads[i].tid);

		if(threads[i].cleanup != 0x0)
			threads[i].cleanup();
	}

	DEBUG(1, "terminating child processes\n");
	brickos_destroy_childs();

	term_default();

	// remove shared memory segments that are left from uncleanly terminated
	// executions, e.g. killed or segfaulted
	DEBUG(1, "remove left-over shared memory segments\n");
	system("ipcrm --all=shm");
}

static void process_info(void){
	printf(
		"Process information\n"
		"  %s pid: %u\n"
		"  kernel pid: %u\n"
		"  app pid: %u\n"
		, PROGNAME, getpid()
		, KERNEL->pid
		, APP->pid
	);
}
