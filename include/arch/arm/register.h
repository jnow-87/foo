/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARM_REGISTER_H
#define ARM_REGISTER_H


/* macros */
#define mreg_r(reg_addr)			(*((volatile register_t*)(reg_addr)))
#define mreg_w(reg_addr, reg_val)	(mreg_r(reg_addr) = reg_val)


#endif // ARM_REGISTER_H
