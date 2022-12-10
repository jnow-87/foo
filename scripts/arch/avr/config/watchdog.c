/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <float.h>
#include <math.h>
#include "avrconfig.h"


/* macros */
// ensure CONFIG-variables have viable values
#if !defined(CONFIG_KTIMER_CYCLETIME_US)
# define CONFIG_KTIMER_CYCLETIME_US	(2048 * 1000000.0 / CONFIG_WATCHDOG_CLOCK_HZ)
# define CONFIG_KTIMER_ERR_MAX		0
#endif


/* local/static prototypes */
static unsigned int cycle_time_multiple(float cycle_time_us, float prescale);
static float err_abs(float cycle_time_us, float prescale, unsigned int mul);


/* global functions */
int config_watchdog(void){
	float err_min = FLT_MAX;
	unsigned int ps_min = 0;
	unsigned int mul;
	float err,
		  err_per;


	if(arg.verbose){
		printf("target kernel timer cycle time: %.3fms\n", CONFIG_KTIMER_CYCLETIME_US / 1000.0);
		printf("max. kernel timer error: %u%%\n", CONFIG_KTIMER_ERR_MAX);
		printf("watchdog base clock: %uHz\n", CONFIG_WATCHDOG_CLOCK_HZ);
		printf("%9.9s %19.19s %6.6s %21.21s\n", "", "watchdog   ", "", "error    ");
		printf("%9.9s %9.9s %9.9s %6.6s %10.10s %10.10s\n", "prescale", "[Hz]", "[ms]", "factor", "[ms]", "%");
	}

	/* identify watchdog prescale value that results in the minimal deviation
	 * between scheduler and watchdog frequency */
	for(unsigned int ps=1048576; ps>=2048; ps/=2){
		mul = cycle_time_multiple(CONFIG_KTIMER_CYCLETIME_US, ps);
		err = err_abs(CONFIG_KTIMER_CYCLETIME_US, ps, mul);

		// update result if error is lower
		if(fabs(err) < err_min){
			ps_min = ps;
			err_min = fabs(err);
		}

		if(arg.verbose)
			printf("%9u %9.2f %9.2f %6u %10.2f %10.2f\n",
				ps,
				CONFIG_WATCHDOG_CLOCK_HZ / (float)ps,
				(float)ps * 1000 / CONFIG_WATCHDOG_CLOCK_HZ,
				mul,
				err * 1000,
				err * 100 * (1000000.0 / CONFIG_KTIMER_CYCLETIME_US)
			);
	}

	if(ps_min == 0){
		fprintf(stderr, "error: invalid prescaler value\n");
		return -1;
	}

	if(arg.verbose)
		printf("\nselected prescale value: %u\n\n", ps_min);

	/* write config */
	CONFIG_PRINT(WATCHDOG_PRESCALE, ps_min, "%u");

	mul = cycle_time_multiple(CONFIG_KTIMER_CYCLETIME_US, ps_min);
	err = err_abs(CONFIG_KTIMER_CYCLETIME_US, ps_min, mul);
	err_per = err * 100 * (1000000.0 / CONFIG_KTIMER_CYCLETIME_US);

	if(fabs(err_per) > CONFIG_KTIMER_ERR_MAX){
		fprintf(stderr, "error: kernel timer error higher than %u%%\n", CONFIG_KTIMER_ERR_MAX);
		return -1;
	}

	CONFIG_PRINT(KTIMER_FACTOR, mul, "%u");
	CONFIG_PRINT(KTIMER_ERROR_US, err * 1000000, "%.0f");

	return 0;
}


/* local functions */
/**
 * \brief	compute the number of watchdog cycles required to reach cycle_time_us
 * 			with the given prescaler value
 *
 * \param	cycle_time_us	target cycle time
 * \param	prescale		watchdog prescaler value
 *
 * \return	number of cycles required (at least 1)
 */
static unsigned int cycle_time_multiple(float cycle_time_us, float prescale){
	unsigned int mul;


	mul = (cycle_time_us / 1000000.0) / (prescale / CONFIG_WATCHDOG_CLOCK_HZ);

	return (mul == 0 ? 1 : mul);
}

/**
 * \brief	compute the absolute error between target cycle time and the actual
 * 			cycle time obtained with the watchdog cycle time, the given cycle time
 * 			multiple and watchdog prescaler value
 *
 * \param	cycle_time_us	target cycle time
 * \param	prescale		watchdog prescaler value
 * \param	mul				cycle time multiple
 *
 * \return	error value
 */
static float err_abs(float cycle_time_us, float prescale, unsigned int mul){
	return (mul * prescale / CONFIG_WATCHDOG_CLOCK_HZ) - (cycle_time_us / 1000000.0);
}
