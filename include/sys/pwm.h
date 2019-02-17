/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_PWM_H
#define SYS_PWM_H


/* types */
typedef enum{
	PWM_FAST = 1,
	PWM_PHASECORRECT,
} pwm_mode_t;

typedef enum{
	PWM_PRES_0 = 0,
	PWM_PRES_1,
	PWM_PRES_2,
	PWM_PRES_4,
	PWM_PRES_8,
	PWM_PRES_16,
	PWM_PRES_32,
	PWM_PRES_64,
	PWM_PRES_128,
	PWM_PRES_256,
	PWM_PRES_512,
	PWM_PRES_1024,
	PWM_PRES_MAX
} pwm_pres_t;

typedef struct{
	pwm_mode_t mode;

	unsigned int base_clock_khz;
	pwm_pres_t prescaler;

	unsigned int max;
} pwm_cfg_t;


#endif // SYS_PWM_H
