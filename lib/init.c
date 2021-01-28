/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <lib/init.h>
#include <lib/stdlib.h>
#include <sys/compiler.h>
#include <sys/thread.h>


/* types */
typedef void (*std_init_call_t)(void);


/* prototypes */
int main();


/* external variables */
extern std_init_call_t __std_preinit_array_base[],
					   __std_preinit_array_end[],
					   __std_init_array_base[],
					   __std_init_array_end[],
					   __std_fini_array_base[],
					   __std_fini_array_end[];

extern lib_init_call_t __lib_init0_base[],
					   __lib_init0_end[],
					   __lib_init1_base[],
					   __lib_init1_end[];


/* local/static prototypes */
static int main_arg(int argc, char *args);
static int args_split(char *args);

static void exec_std_call(std_init_call_t *base, std_init_call_t *end);
static void exec_lib_call(lib_init_call_t *base, lib_init_call_t *end);

/* global functions */
/**
 * \brief	Program entry point. Based on the given entry point either the main function
 * 			or a thread functions is called. If entry is set to _start the main function
 * 			is called, otherwise the functions specified through entry is called.
 * 			If the main functions is called program initialisations are performed.
 *
 * \param	entry	specify the target function
 * 					if set to_start the main function is called
 * 					otherwise the function specified through entry is called
 *
 * \param	arg		respective thread argument
 * 					if entry is set to _start arg is assumed to contain the argument string
 * 					otherwise arg is passed to the target function as is
 */
void _start(thread_entry_t entry, void *arg){
	int r;


	/* call specified function */
	if((void*)entry == (void*)_start){
		// init
		exec_std_call(__std_preinit_array_base, __std_preinit_array_end);
		exec_std_call(__std_init_array_base, __std_init_array_end);

		exec_lib_call(__lib_init0_base, __lib_init0_end);
		exec_lib_call(__lib_init1_base, __lib_init1_end);

		// main
		r = main_arg(args_split(arg), arg);

		// fini
		exec_std_call(__std_fini_array_base, __std_fini_array_end);

		_exit(r, true);
	}
	else{
		_exit((*entry)(arg), false);
	}
}


/* local functions */
static int main_arg(int argc, char *args){
	char *argv[argc + 1];
	int i,
		j;


	/* assign argument strings */
	j = 1;
	argv[0] = args;
	argv[argc] = 0x0;

	for(i=0; j<argc; i++){
		if(args[i] == 0)
			argv[j++] = args + i + 1;
	}

	/* call */
	return main(argc, argv);
}

/**
 * \brief	Parse argument string args, identifying non-empty strings. Non-empty strings are character
 * 			sequences that are separated by '\t' or ' ' and quoted character sequences. The function
 * 			truncates all unquoted '\t' and ' ' and replaces them by a single 0.
 *
 * \post	args is a string containing multiple 0 characters used to separate sub-strings
 *
 * \param	args	string to be parsed
 *
 * \return	number of sub-strings in argc
 */
static int args_split(char *args){
	unsigned int i, j;
	int argc;
	char c;



	argc = 0;
	j = 0;
	c = ' ';

	for(i=0; args[i]!=0; i++){
		if((args[i] == '"' && c == '"')										// end of quoted section
		|| ((args[i] == ' ' || args[i] == '\t') && c != '"' && c != ' ')	// end of string
		){
			args[j++] = 0;
			argc++;

			c = ' ';
		}
		else if(args[i] == '"'){											// start of quoted string
			c = '"';
		}
		else if((args[i] != ' ' && args[i] != '\t') || c != ' '){			// inside string
			// check for escaped \ and "
			if(args[i] == '\\' && (args[i + 1] == '\\' || args[i + 1] == '"'))
				i++;

			args[j++] = args[i];

			// update last character if
			// not inside a quoted string
			if(c != '"')
				c = args[i];
		}
	}

	if(j > 0 && args[j - 1] != 0){
		args[j] = 0;
		argc++;
	}

	return argc;
}

static void exec_std_call(std_init_call_t *base, std_init_call_t *end){
	for(; base!=end; base++){
		(*base)();
	}
}

static void exec_lib_call(lib_init_call_t *base, lib_init_call_t *end){
	for(; base!=end; base++){
		if((*base)() != 0)
			exit(-errno);
	}
}
