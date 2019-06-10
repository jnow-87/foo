/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <arch/avr/ctxsanity.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <kernel/kprintf.h>
#include <sys/register.h>
#include <sys/syscall.h>
#include <sys/list.h>


/* macros */
#define IN_PROC_STACK(tc) 		((unsigned int)(tc) >= (CONFIG_KERNEL_PROC_BASE & 0xffff) && (unsigned int)(tc) <= (CONFIG_KERNEL_PROC_BASE & 0xffff) + CONFIG_KERNEL_PROC_SIZE - 1)
#define IN_KERNEL_STACK(tc)		((unsigned int)(tc) >= (CONFIG_KERNEL_STACK_BASE & 0xffff) && (unsigned int)(tc) <= (CONFIG_KERNEL_STACK_BASE & 0xffff) + CONFIG_KERNEL_STACK_SIZE - 1)
#define DIFF_INDICATOR(tc0, tc1, reg) (((tc0)->reg != (tc1)->reg) ? '*' : ' ')


/* local/static prototypes */
static void print_reg(int_num_t int_num, int sc_num, struct thread_ctx_t *tc);
static void print_reg_cmp(int_num_t int_num, int sc_num, struct thread_ctx_t *tc0, struct thread_ctx_t *tc1);


/* external variables */
extern process_t *process_table;


/* global functions */
int ctxsanity_check_all_ctx(char const *msg, int int_num, int sc){
	int r;
	process_t *this_p;
	thread_t const *this_t;
	thread_ctx_t *tc,
				 *tc_cmp;


	r = 0;

	list_for_each(process_table, this_p){
		for(this_t=this_p->threads; this_t!=0x0; this_t=this_t->next){
			tc_cmp = this_t->ctx_stack_cmp;

			for(tc=this_t->ctx_stack; tc!=0; tc=tc->next){
				r |= ctxsanity_check_ctx(msg, int_num, sc, tc, tc_cmp, this_t, 0x0);
				tc_cmp = tc_cmp->next;
			}
		}
	}

	return r;
}

int ctxsanity_check_ctx(char const *msg, int int_num, int sc, thread_ctx_t *tc, thread_ctx_t *tc_cmp, thread_t const *this_t_entry, thread_t const *this_t_exit){
	if(memcmp((void*)tc + sizeof(thread_ctx_t*), (void*)tc_cmp + sizeof(thread_ctx_t*), sizeof(thread_ctx_t) - sizeof(thread_ctx_t*)) != 0){
		INFO("\n");
		FATAL("%s thread context has changed for %s.%d\n", msg, this_t_entry->parent->name, this_t_entry->tid);

		if(this_t_entry && this_t_exit){
			INFO("        entry         exit\n");
			INFO("thread: %s.%d, %s.%d\n", this_t_entry->parent->name, this_t_entry->tid, this_t_exit->parent->name, this_t_exit->tid);
			INFO("stack:  %#x, %#x\n", this_t_entry->stack->phys_addr, this_t_exit->stack->phys_addr);
		}

		print_reg_cmp(int_num, sc, tc, tc_cmp);

		return -1;
	}

	return 0;
}

int ctxsanity_check_stack_addr(char const *msg, int int_num, int sc, struct thread_ctx_t *tc, thread_t const *this_t){
	if(this_t->parent->pid != 0 && !IN_PROC_STACK(tc)){
		FATAL("%s %d %d process stack out of range: %s.%d %#x\n", msg, int_num, sc, this_t->parent->name, this_t->tid, tc);

		if(IN_KERNEL_STACK(tc))
			FATAL("but in kernel range\n");

		print_reg(int_num, sc, tc);

		return -1;
	}

	if(this_t->parent->pid == 0 && !IN_KERNEL_STACK(tc)){
		FATAL("%s %d %d kernel stack out of range: %s.%d %#x\n", msg, int_num, sc, this_t->parent->name, this_t->tid, tc);

		if(IN_PROC_STACK(tc))
			FATAL("but in process range\n");

		print_reg(int_num, sc, tc);

		return -1;
	}

	return 0;
}

int ctxsanity_sc_num(void){
	sc_arg_t *arg;


	/* acquire parameter */
	// get address from GPIO registers 0/1
	arg = (sc_arg_t*)(mreg_r(GPIOR0) | (mreg_r(GPIOR1) << 8));
	return arg->num;
}


/* local functions */
static void print_reg(int_num_t int_num, int sc_num, struct thread_ctx_t *tc){
	unsigned int i,
				 j;


	kprintf(KMSG_ANY, "int %u, sc %u\n"
		 "%20.20s: %p\n"
		 "%20.20s: %#2.2x\n"
		 "%20.20s: %#2.2x\n"
		 "%20.20s: %#2.2x\n"
		 "%20.20s: %#2.2x\n"
		 "%20.20s: %p\n"
		 "%20.20s: %p\n"
		 ,
		 int_num,
		 sc_num,
		 "ctx", tc,
		 "SREG", tc->sreg,
		 "MCUSR", tc->mcusr,
		 "GPIOR0", tc->gpior[0],
		 "GPIOR1", tc->gpior[1],
		 "interrupted at", ((lo8(tc->ret_addr) << 8) | hi8(tc->ret_addr)) * 2,
		 "interrupt vector", ((lo8(tc->int_vec_addr) << 8) | hi8(tc->int_vec_addr)) * 2
	);

	for(i=0; i<32; i++){
		j = i / 4 + (i % 4) * 8;
		kprintf(KMSG_ANY, "\t%2.2u: %#2.2x", j, tc->gpr[j]);

		if(i % 4 == 3)
			kprintf(KMSG_ANY, "\n");
	}

	kprintf(KMSG_ANY, "\n");
}

static void print_reg_cmp(int_num_t int_num, int sc_num, struct thread_ctx_t *tc0, struct thread_ctx_t *tc1){
	unsigned int i,
				 j;


	kprintf(KMSG_ANY, "int %u, sc %u, running %s.%d\n"
		 "%20.20s: %p %p\n"
		 "%20.20s: %#2.2x %#2.2x%c\n"
		 "%20.20s: %#2.2x %#2.2x%c\n"
		 "%20.20s: %#2.2x %#2.2x%c\n"
		 "%20.20s: %#2.2x %#2.2x%c\n"
		 "%20.20s: %p %p%c\n"
		 "%20.20s: %p %p%c\n"
		 ,
		 int_num,
		 sc_num,
		 sched_running()->parent->name, sched_running()->tid,
		 "ctx", tc0, tc1,
		 "SREG", tc0->sreg, tc1->sreg, DIFF_INDICATOR(tc0, tc1, sreg),
		 "MCUSR", tc0->mcusr, tc1->mcusr, DIFF_INDICATOR(tc0, tc1, mcusr),
		 "GPIOR0", tc0->gpior[0], tc1->gpior[0], DIFF_INDICATOR(tc0, tc1, gpior[0]),
		 "GPIOR1", tc0->gpior[1], tc1->gpior[1], DIFF_INDICATOR(tc0, tc1, gpior[1]),
		 "interrupted at", ((lo8(tc0->ret_addr) << 8) | hi8(tc0->ret_addr)) * 2, ((lo8(tc1->ret_addr) << 8) | hi8(tc1->ret_addr)) * 2, DIFF_INDICATOR(tc0, tc1, ret_addr),
		 "interrupt vector", ((lo8(tc0->int_vec_addr) << 8) | hi8(tc0->int_vec_addr)) * 2, ((lo8(tc1->int_vec_addr) << 8) | hi8(tc1->int_vec_addr)) * 2, DIFF_INDICATOR(tc0, tc1, int_vec_addr)
	);

	for(i=0; i<32; i++){
		j = i / 4 + (i % 4) * 8;
		kprintf(KMSG_ANY, "\t%2.2u: %#2.2x %#2.2x%c", j, tc0->gpr[j], tc1->gpr[j], DIFF_INDICATOR(tc0, tc1, gpr[j]));

		if(i % 4 == 3)
			kprintf(KMSG_ANY, "\n");
	}

	kprintf(KMSG_ANY, "\n");
}
