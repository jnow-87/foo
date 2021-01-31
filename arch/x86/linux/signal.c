/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <sys/types.h>


/* macros */
#define SIG_FL_USE_SIGRETURN	0x04000000


/* types */
typedef struct{
	lnx_sig_hdlr_t hdlr;
	unsigned long int flags;

	void (*sigreturn)(void);

	uint64_t mask[16];
} lnx_sigaction_t;


/* local/static prototypes */
static void sigreturn_rt(void) __attribute__((naked));


/* global functions */
void lnx_sigset(int sig, lnx_sig_hdlr_t hdlr){
	long int r;
	lnx_sigaction_t act = { 0x0 };


	act.hdlr = hdlr;
	act.sigreturn = &sigreturn_rt;
	act.flags |= SIG_FL_USE_SIGRETURN;

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
