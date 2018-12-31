/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <sys/syscall.h>
#include <sys/signal.h>
#include <sys/thread.h>
#include <sys/errno.h>
#include <lib/init.h>
#include <lib/stdlib.h>
#include <lib/signal.h>


/* local/static prototypes */
static void signal_hdlr(thread_entry_t entry, void *arg);


/* static variables */
signal_hdlr_t hdlrs[SIG_MAX] = { 0x0 };


/* global functions */
signal_hdlr_t signal(signal_t sig, signal_hdlr_t hdlr){
	if(sig >= SIG_MAX)
		goto_errno(err, E_INVAL);

	if(hdlrs[sig] == 0x0)
		hdlrs[sig] = hdlr;

	return hdlrs[sig];


err:
	return 0x0;
}

int signal_send(signal_t sig, pid_t pid, tid_t tid){
	sc_signal_t p;


	p.sig = sig;
	p.hdlr = signal_hdlr;
	p.pid = pid;
	p.tid = tid;

	if(sc(SC_SIGSEND, &p) != E_OK)
		return -1;
	return 0;
}


/* local functions */
static void signal_hdlr(thread_entry_t entry, void *arg){
	signal_t sig;


	sig = (signal_t)arg;

	if(hdlrs[sig] != 0x0){
		hdlrs[sig](sig);
		(void)sc(SC_SIGRETURN, arg);

		// SC_SIGRETURN should never return
		// just in case, fall through to exit
	}

	exit(7);
}
