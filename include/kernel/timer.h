/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H


#include <config/config.h>


/* types */
typedef void (*ktimer_hdlr_t)(void *data);

typedef struct ktimer_t{
	struct ktimer_t *prev,
					*next;

	size_t ticks;
	ktimer_hdlr_t hdlr;
	void *data;
} ktimer_t;



/* prototypes */
void ktimer_tick(void);

void ktimer_register(ktimer_t *timer, uint32_t period_us, ktimer_hdlr_t hdlr, void *data);
void ktimer_release(ktimer_t *timer);


/* disabled calls */
#ifndef CONFIG_KERNEL_TIMER
#define ktimer_tick()
#endif


#endif // KERNEL_TIMER_H
