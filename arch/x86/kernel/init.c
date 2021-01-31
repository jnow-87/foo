/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* types */
typedef void (*std_init_call_t)(void);


/* prototypes */
static void exec_call(std_init_call_t *base, std_init_call_t *end);


/* external variables */
extern std_init_call_t __std_preinit_array_base[],
					   __std_preinit_array_end[],
					   __std_init_array_base[],
					   __std_init_array_end[],
					   __std_fini_array_base[],
					   __std_fini_array_end[];


/* global functions */
void x86_std_init(void){
	exec_call(__std_preinit_array_base, __std_preinit_array_end);
	exec_call(__std_init_array_base, __std_init_array_end);
}

void x86_std_fini(void){
	exec_call(__std_fini_array_base, __std_fini_array_end);
}


/* local functions */
static void exec_call(std_init_call_t *base, std_init_call_t *end){
	for(; base!=end; base++){
		(*base)();
	}
}
