/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_SIGNAL_H
#define LIB_SIGNAL_H


#include <sys/signal.h>
#include <sys/process.h>
#include <sys/thread.h>


/* types */
typedef void (*signal_hdlr_t)(signal_t sig);


/* prototypes */
signal_hdlr_t signal(signal_t sig, signal_hdlr_t hdlr);
int signal_send(signal_t sig, pid_t pid, tid_t tid);


#endif // LIB_SIGNAL_H
