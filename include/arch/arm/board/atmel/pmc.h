/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ATMEL_PMC_H
#define ATMEL_PMC_H


/* prototypes */
int pmc_init(void);
int pmc_per_enable(unsigned int pid, unsigned int src, unsigned int div);
int pmc_per_disable(unsigned int pid);


#endif // ATMEL_PMC_H
