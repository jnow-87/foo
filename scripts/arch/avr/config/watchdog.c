#include <arch/avr/atmega.h>
#include <config/config.h>
#include <float.h>
#include <math.h>
#include "avrconfig.h"


/* macros */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


/* local/static prototypes */
static unsigned int cycle_time_multiple(float cycle_time_us, float prescale);
static float err_abs(float cycle_time_us, float prescale, unsigned int mul);


/* global functions */
int config_watchdog(void){
	unsigned int ps,
				 ps_min,
				 mul;
	float tgt_cycle_time;
	float err,
		  err_min,
		  err_per;


	err_min = FLT_MAX;
	tgt_cycle_time = MIN(CONFIG_SCHED_CYCLETIME_US, CONFIG_KTIMER_CYCLETIME_US);

	if(arg.verbose){
		printf("\ntarget scheduler cycle time: %.3fms\n", CONFIG_SCHED_CYCLETIME_US / 1000.0);
		printf("target kernel timer cycle time: %.3fms\n", CONFIG_KTIMER_CYCLETIME_US / 1000.0);
		printf("target cycle: %.3fms\n", tgt_cycle_time / 1000.0);
		printf("\nmax. scheduler timer error: %u%%\n", CONFIG_SCHED_ERR_MAX);
		printf("max. kernel timer error: %u%%\n", CONFIG_KTIMER_ERR_MAX);
		printf("\nwatchdog base clock: %uHz\n", WATCHDOG_HZ);
		printf("\n%9.9s %19.19s %6.6s %21.21s\n", "", "watchdog   ", "", "error    ");
		printf("%9.9s %9.9s %9.9s %6.6s %10.10s %10.10s\n", "prescale", "[Hz]", "[ms]", "factor", "[ms]", "%");
	}

	/* identify watchdog prescale value that results in the minimal deviation
	 * between scheduler and watchdog frequency */
	for(ps=1048576; ps>=2048; ps/=2){
		mul = cycle_time_multiple(tgt_cycle_time, ps);
		err = err_abs(tgt_cycle_time, ps, mul);

		// update result if error is lower
		if(fabs(err) < err_min){
			ps_min = ps;
			err_min = fabs(err);
		}

		if(arg.verbose)
			printf("%9u %9.2f %9.2f %6u %10.2f %10.2f\n",
				ps,
				WATCHDOG_HZ / (float)ps,
				(float)ps * 1000 / WATCHDOG_HZ,
				mul,
				err * 1000,
				err * 100 * (1000000.0 / tgt_cycle_time)
			);
	}

	if(arg.verbose)
		printf("\nselected prescale value: %u\n\n", ps_min);

	/* write config */
	CONFIG_PRINT(WATCHDOG_PRESCALE, ps_min, "%u");

	// scheduler
	mul = cycle_time_multiple(CONFIG_SCHED_CYCLETIME_US, ps_min);
	err = err_abs(CONFIG_SCHED_CYCLETIME_US, ps_min, mul);
	err_per = err * 100 * (1000000.0 / CONFIG_SCHED_CYCLETIME_US);

	if(fabs(err_per) > CONFIG_SCHED_ERR_MAX){
		fprintf(stderr, "error: scheduler timer error higher than %u%%\n", CONFIG_SCHED_ERR_MAX);
		return -1;
	}

	CONFIG_PRINT(SCHED_FACTOR, mul, "%u");
	CONFIG_PRINT(SCHED_ERROR_US, err * 1000000, "%.0f");

	// timer
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


	mul = (cycle_time_us / 1000000.0) / (prescale / WATCHDOG_HZ);

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
	return (mul * prescale / WATCHDOG_HZ) - (cycle_time_us / 1000000.0);
}
