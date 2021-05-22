/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <brickos/brickos.h>
#include <brickos/child.h>
#include <user/opts.h>


/* local/static prototypes */
static void argv_sort(char **argv);


/* global variables */
child_t *brickos_childs[BOS_NCHILDS] = { 0x0 };


/* global functions */
int brickos_init_childs(void){
	unsigned int i;
	char *argv[] = {
		opts.kernel_image,
		opts.verbosity > 0 ? "-v" : "",
		(opts.app_mode & AM_INTERACTIVE) ? "-i" : "",
		0x0
	};


	argv_sort(argv);

	KERNEL = child_create("kernel");
	APP = child_create("app");

	for(i=0; i<BOS_NCHILDS; i++){
		if(brickos_childs[i] == 0x0)
			goto err;
	}

	argv[0] = opts.kernel_image;
	child_add_pipe(KERNEL, CONFIG_TEST_INT_HW_PIPE_RD, CONFIG_TEST_INT_HW_PIPE_WR);
	child_add_pipe(KERNEL, CONFIG_TEST_INT_USR_PIPE_RD, CONFIG_TEST_INT_USR_PIPE_WR);
	child_fork(KERNEL, argv);

	argv[0] = opts.app_binary;
	child_add_pipe(APP, CONFIG_TEST_INT_HW_PIPE_RD, CONFIG_TEST_INT_HW_PIPE_WR);
	child_add_pipe(APP, CONFIG_TEST_INT_USR_PIPE_RD, CONFIG_TEST_INT_USR_PIPE_WR);
	child_fork(APP, argv);

	return 0;


err:
	brickos_destroy_childs();

	return -1;
}

void brickos_destroy_childs(void){
	unsigned int i;


	for(i=0; i<BOS_NCHILDS; i++){
		if(brickos_childs[i])
			child_destroy(brickos_childs[i]);
	}
}


/* local functions */
static void argv_sort(char **argv){
	size_t i,
		   j;


	j = 0;

	for(i=0; argv[i]!=0x0; i++){
		if(argv[i][0] != 0)
			continue;

		for(j=i; argv[j]!=0x0; j++){
			if(argv[j][0] != 0){
				argv[i] = argv[j];
				argv[j] = "";
				break;
			}
		}

		if(argv[j] == 0x0){
			argv[i] = 0x0;
			break;
		}
	}
}
