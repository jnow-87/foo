/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <kernel/ipi.h>
#include <sys/types.h>
#include <test/test.h>


/* macros */
#define N	3


/* local/static prototypes */
static void ipi_hdlr(void *payload);


/* static variables */
static uint8_t recv_cnt[2] = { 0 };
static int8_t recv_data[2][N] = { 0 };
static int send_status = 0;


/* local functions */
TEST(ipi_send){
	int r = 0;
	bool istate;


	/* send messages to core 1 */
	for(int8_t i=0; i<N; i++)
		r += TEST_INT_EQ(ipi_send(1, ipi_hdlr, &i, sizeof(int8_t)), 0);

	/* wait to receive core 1 answers */
	istate = int_enable(true);

	while(recv_cnt[0] != N)
		core_sleep();

	int_enable(istate);

	/* check results */
	r += TEST_INT_EQ(recv_cnt[0], N);
	r += TEST_INT_EQ(recv_cnt[1], N);
	r += TEST_INT_EQ(send_status, 0);

	for(int8_t i=0; i<N; i++){
		r += TEST_INT_EQ(recv_data[0][i], -i);
		r += TEST_INT_EQ(recv_data[1][i], i);
	}

	return -r;
}

static void ipi_hdlr(void *payload){
	int8_t p = *((int8_t*)payload);


	recv_data[PIR][recv_cnt[PIR]] = p;
	recv_cnt[PIR]++;

	p = -p;

	if(PIR != 0)
		send_status += ipi_send(0, ipi_hdlr, &p, sizeof(int8_t));
}
