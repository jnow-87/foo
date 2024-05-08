/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H


#include <config/config.h>
#include <sys/time.h>


/* types */
typedef void (*ktimer_hdlr_t)(void *payload);

typedef struct ktimer_t{
	struct ktimer_t *prev,
					*next;

	size_t ticks,
		   base;

	ktimer_hdlr_t hdlr;
	void *payload;
} ktimer_t;


/* prototypes */
void ktimer_start(ktimer_t *timer, uint32_t cycle_time_us, ktimer_hdlr_t hdlr, void *payload, bool periodic);
void ktimer_abort(ktimer_t *timer);

uint32_t ktimer_ms(void);
void ktimer_time(time_t *t);


#endif // KERNEL_TIMER_H
