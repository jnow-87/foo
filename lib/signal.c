/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/syscall.h>
#include <sys/syscall.h>
#include <sys/signal.h>
#include <sys/thread.h>
#include <sys/errno.h>
#include <lib/init.h>
#include <lib/stdlib.h>
#include <lib/signal.h>
#include <lib/unistd.h>


/* local/static prototypes */
static int signal_hdlr(void *arg);
static void action_exit(signal_t sig);


/* static variables */
signal_hdlr_t hdlrs[SIG_MAX] = { 0x0 };


/* global functions */
signal_hdlr_t signal(signal_t sig, signal_hdlr_t hdlr){
	if(sig >= SIG_MAX || sig == SIG_KILL)
		goto_errno(err, E_INVAL);

	if(hdlrs[sig] == 0x0 || hdlr == 0x0)
		hdlrs[sig] = hdlr;

	return hdlrs[sig];


err:
	return 0x0;
}

int signal_send(signal_t sig, pid_t pid, tid_t tid){
	sc_signal_t p;


	p.sig = sig;
	p.pid = pid;
	p.tid = tid;

	if(sc(SC_SIGSEND, &p) != 0)
		return -1;

	return 0;
}


/* local functions */
static int init(void){
	sc_signal_t p;
	process_info_t pinfo;


	if(process_info(&pinfo) != 0)
		return -errno;

	hdlrs[SIG_INT] = action_exit;
	hdlrs[SIG_KILL] = action_exit;
	hdlrs[SIG_TERM] = action_exit;

	p.pid = pinfo.pid;
	p.hdlr = signal_hdlr;

	return sc(SC_SIGREGISTER, &p);
}

lib_init(1, init);

static int signal_hdlr(void *arg){
	char dummy;
	signal_t sig;


	sig = (signal_t)arg;

	if(hdlrs[sig] != 0x0){
		hdlrs[sig](sig);
		(void)sc(SC_SIGRETURN, &dummy);

		// SC_SIGRETURN should never return
		// just in case, fall through to exit
	}

	return_errno(E_INVAL);
}

static void action_exit(signal_t sig){
	_exit(-sig, false);
}
