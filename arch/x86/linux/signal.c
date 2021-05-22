/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <sys/types.h>
#include <sys/string.h>


/* macros */
#define SIG_FL_USE_SIGRETURN	0x04000000


/* types */
typedef struct{
	lnx_sig_hdlr_t hdlr;
	unsigned long int flags;

	void (*sigreturn)(void);

	lnx_sigset_t mask;
} lnx_sigaction_t;


/* local/static prototypes */
static void sigreturn_rt(void) __attribute__((naked));


/* global functions */
void lnx_sigaction(int sig, lnx_sig_hdlr_t hdlr, lnx_sigset_t *blocked){
	long int r;
	lnx_sigaction_t act = { 0x0 };


	act.hdlr = hdlr;
	act.sigreturn = &sigreturn_rt;
	act.flags |= SIG_FL_USE_SIGRETURN;

	if(blocked)
		memcpy(&act.mask, blocked, sizeof(lnx_sigset_t));

	r = lnx_syscall(LNX_SYS_SIGACTION,
		(unsigned long int[6]){
			sig,
			(unsigned long int)&act,
			0,
			8,
			0,
			0
		},
		1
	);

	if(r != 0)
		LNX_SYSCALL_ERROR_EXIT("%d\n", r);
}

void lnx_sigaddset(lnx_sigset_t *set, int sig){
	size_t i;


	i = --sig / 64;
	set->data[i] |= ((uint64_t)0x1) << (sig % 64);
}

void lnx_kill(int pid, int sig){
	long int r;


	r = lnx_syscall(LNX_SYS_KILL, (unsigned long int[6]){pid, sig, 0, 0, 0, 0}, 1);

	if(r != 0)
		LNX_SYSCALL_ERROR_EXIT("%d\n", r);
}

void lnx_pause(void){
	(void)lnx_syscall(LNX_SYS_PAUSE, (unsigned long int[6]){0, 0, 0, 0, 0, 0}, 0);
}


/* local functions */
static void sigreturn_rt(void){
	asm volatile(
		"mov	%[num], %%rax\n"
		"syscall\n"
		:
		: [num] "i" (LNX_SYS_SIGRETURN_RT)
	);
}
