#include <arch/avr/atmega.h>
#include <config/config.h>
#include <float.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>


/* macros */
#define CONFIG_PRINT(var, value, fmt) \
	fprintf(arg.ofile, "#define AVRCONFIG_" #var " " fmt "\n", (value))


/* types */
typedef struct{
	FILE *ofile;
	char *ofile_name;
	int verbose;
} arg_t;


/* global variables */
arg_t arg = { 0 };


/* local/static prototypes */
int arg_parse(int argc, char **argv);
int config_watchdog(void);


/* global functions */
int main(int argc, char **argv){
	int r;


	if(arg_parse(argc, argv) < 0)
		return 1;

	printf("generating avr config header \"%s\"\n", arg.ofile_name);

	/* compute config variables */
	r = 0;

	r |= config_watchdog();

	/* close output file */
	fclose(arg.ofile);

	if(r == 0)
		return 0;

	unlink(arg.ofile_name);

	return 3;
}


/* local functions */
int arg_parse(int argc, char **argv){
	int i;


	for(i=1; i<argc; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
			case 'v':
				arg.verbose = 1;
				break;

			default:
				fprintf(stderr, "invalid argument \"%s\"\n", argv[i]);
				return -1;
			}
		}
		else{
			arg.ofile_name = argv[i];
			arg.ofile = fopen(argv[i], "w");

			if(arg.ofile == 0)
				fprintf(stderr, "unable to open output header \"%s\"\n", strerror(errno));
		}
	}

	if(arg.ofile == 0){
		fprintf(stderr, "missing output file\n");
		return -1;
	}

	return 0;
}

int config_watchdog(void){
	unsigned int ps,
				 ps_min,
				 mul,
				 mul_min;
	float err,
		  err_min,
		  err_per;


	err_min = FLT_MAX;

	if(arg.verbose){
		printf("\ntarget scheduler frequency: %uHz\n", CONFIG_SCHED_HZ);
		printf("max. timer error: %u%%\n", CONFIG_SCHED_ERR_MAX);
		printf("watchdog base clock: %uHz\n", WATCHDOG_HZ);
		printf("\n%9.9s %19.19s %6.6s %21.21s\n", "", "watchdog   ", "", "error    ");
		printf("%9.9s %9.9s %9.9s %6.6s %10.10s %10.10s\n", "prescale", "[Hz]", "[ms]", "factor", "[ms]", "%");
	}

	/* identify watchdog prescale value that results in the minimal deviation
	 * between scheduler and watchdog frequency */
	for(ps=1048576; ps>=2048; ps/=2){
		// number of watchdog ticks to trigger one scheduler tick
		mul = (1.0 / CONFIG_SCHED_HZ) / ((float)ps / WATCHDOG_HZ);

		if(mul == 0)
			mul = 1;

		// absolute error for the current prescale value
		err = (1.0 / CONFIG_SCHED_HZ) - (mul * (float)ps / WATCHDOG_HZ);

		// update result if error is lower
		if(fabs(err) < err_min){
			ps_min = ps;
			err_min = fabs(err);
			mul_min = mul;
		}

		if(arg.verbose)
			printf("%9u %9.2f %9.2f %6u %10.2f %10.2f\n",
				ps,
				WATCHDOG_HZ / (float)ps,
				(float)ps * 1000 / WATCHDOG_HZ,
				mul,
				err * 1000,
				err * 100 * CONFIG_SCHED_HZ
			);
	}

	if(arg.verbose)
		printf("\nselected prescale value: %u\n\n", ps_min);

	/* check result */
	err_per = err_min * 100 * CONFIG_SCHED_HZ;

	if(fabs(err_per) > CONFIG_SCHED_ERR_MAX){
		fprintf(stderr, "error: scheduler timer error higher than %u%%\n", CONFIG_SCHED_ERR_MAX);
		return -1;
	}

	/* write config */
	CONFIG_PRINT(WATCHDOG_PRESCALE, ps_min, "%u");
	CONFIG_PRINT(SCHED_FACTOR, mul_min, "%u");
	CONFIG_PRINT(SCHED_ERROR_MS, err_min * 1000, "\"%f\"");

	return 0;
}
