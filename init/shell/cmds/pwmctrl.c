/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/term.h>
#include <sys/pwm.h>
#include <sys/escape.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <shell/cmd.h>


/* macros */
#define LINE_LEN	40


/* types */
typedef enum{
	PRES_DEC = -1,
	PRES_SET = 0,
	PRES_INC = 1,
} pres_act_t;


/* local/static prototypes */
static void update_duty_cycle(unsigned int *cur, int8_t inc, pwm_cfg_t *pwm_cfg, int dev);
static void update_prescaler(pwm_cfg_t *pwm_cfg, pres_act_t action, int dev);
static void update_mode(pwm_cfg_t *pwm_cfg, int dev);
static void update_sample_interval(size_t *cur, int inc);

static void output(char c, pwm_cfg_t *pwm_cfg, unsigned int duty, size_t sample_int);


/* local functions */
static int exec(int argc, char **argv){
	int dev;
	int fd_sample;
	char c;
	unsigned int duty;
	size_t sample_int;
	f_mode_t f_mode;
	pwm_cfg_t cfg;


	if(argc < 3)
		return cmd_help(argv[0], "<pwm device> <sample pin>", "missing arguments", 0);

	/* open pwm and sample pin files */
	dev = open(argv[1], O_RDWR);

	if(dev < 0){
		fprintf(stderr, "open device \"%s\" failed \"%s\"\n", argv[1], strerror(errno));
		return 1;
	}

	fd_sample = open(argv[2], O_RDONLY);

	if(fd_sample < 0){
		fprintf(stderr, "open sample pin \"%s\" failed \"%s\"\n", argv[2], strerror(errno));
		return 1;
	}

	/* make stdin non-blocking */
	fcntl(0, F_MODE_GET, &f_mode, sizeof(f_mode_t));
	f_mode |= O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	/* help */
	printf(
		"'-/+' de/increase duty cycle\n"
		"'p/P' de/increase prescaler\n"
		"'m'   cycle pwm modes\n"
		"'s/S' de/increase sample interval\n\n"
		"'q' quit the test\n\n"
	);

	/* prepare */
	// store terminal position
	printf(STORE_POS);
	fflush(stdout);

	// init variables
	sample_int = 100;
	duty = 0;
	c = 0;

	// disable pwm
	ioctl(dev, IOCTL_CFGRD, &cfg, sizeof(cfg));
	update_duty_cycle(&duty, 0, &cfg, dev);

	/* main loop */
	while(c != 'q'){
		// process user input
		if(read(0, &c, 1) < 0)
			continue;

		switch(c){
		case '+':	update_duty_cycle(&duty, 10, &cfg, dev); break;
		case '-':	update_duty_cycle(&duty, -10, &cfg, dev); break;
		case 'P':	update_prescaler(&cfg, PRES_INC, dev); break;
		case 'p':	update_prescaler(&cfg, PRES_DEC, dev); break;
		case 'm':	update_mode(&cfg, dev); break;
		case 'S':	update_sample_interval(&sample_int, +10); break;
		case 's':	update_sample_interval(&sample_int, -10); break;
		case 'q':	continue;
		};

		// sample pin
		if(read(fd_sample, &c, 1) != 1)
			continue;

		if(c == 0)	output('_', &cfg, duty, sample_int);
		else		output('-', &cfg, duty, sample_int);

		sleep(sample_int, 0);
	}

	/* restore stdin blocking mode */
	f_mode &= ~O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	return 0;
}

command("pwmctrl", exec);

/**
 * \brief	modify the pwm duty cycle depending on inc
 * 			stop the pwm device when reaching a duty cycle of 0
 * 			restart pwm device when starting from a dity cycle of 0
 */
static void update_duty_cycle(unsigned int *cur, int8_t inc, pwm_cfg_t *pwm_cfg, int dev){
	unsigned int t;


	/* enable pwm */
	if(inc > 0 && *cur == 0){
		pwm_cfg->prescaler = PWM_PRES_1;
		update_prescaler(pwm_cfg, PRES_SET, dev);
	}

	/* update duty cycle */
	t = *cur + inc;

	if(inc > 0 && (t > pwm_cfg->max || t < *cur))	*cur = pwm_cfg->max;
	else if(inc < 0 && t > *cur)					*cur = 0;
	else											*cur = t;

	if(write(dev, cur, sizeof(*cur)) <= 0)
		fprintf(stderr, "write to device failed \"%s\"\n", strerror(errno));

	/* disable pwm */
	if(*cur == 0){
		pwm_cfg->prescaler = PWM_PRES_0;
		update_prescaler(pwm_cfg, PRES_SET, dev);
	}
}

/**
 * \brief	find the next supported prescaler value supported by the
 * 			underlying device and defined through action
 */
static void update_prescaler(pwm_cfg_t *pwm_cfg, pres_act_t action, int dev){
	pwm_pres_t t;


	t = pwm_cfg->prescaler;

	switch(action){
	case PRES_INC:
		while(1){
			pwm_cfg->prescaler++;

			if(pwm_cfg->prescaler == PWM_PRES_MAX){
				pwm_cfg->prescaler = t;
				break;
			}

			if(ioctl(dev, IOCTL_CFGWR, pwm_cfg, sizeof(pwm_cfg_t)) == 0)
				break;
		}
		break;

	case PRES_DEC:
		while(1){
			if(pwm_cfg->prescaler == PWM_PRES_0)
				break;

			pwm_cfg->prescaler--;

			if(ioctl(dev, IOCTL_CFGWR, pwm_cfg, sizeof(pwm_cfg_t)) == 0)
				break;
		}
		break;

	case PRES_SET:
		ioctl(dev, IOCTL_CFGWR, pwm_cfg, sizeof(pwm_cfg_t));
		break;
	}
}

/**
 * \brief	cycle through pwm modes 'fast' and 'phase-correct'
 */
static void update_mode(pwm_cfg_t *pwm_cfg, int dev){
	pwm_cfg->mode = (pwm_cfg->mode == PWM_FAST ? PWM_PHASECORRECT : PWM_FAST);
	ioctl(dev, IOCTL_CFGWR, pwm_cfg, sizeof(pwm_cfg_t));
}

/**
 * \brief	modify the sample interval based on inc
 * 			min value: 10
 * 			max value: 300
 */
static void update_sample_interval(size_t *cur, int inc){
	size_t t;


	t = *cur + inc;

	if(inc > 0 && t > 300)						*cur = 300;
	else if(inc < 0 && (t < 10 || t > *cur))	*cur = 10;
	else										*cur = t;
}

/**
 * \brief	output the current test configuration and the level
 * 			of the sampled pin
 */
static void output(char c, pwm_cfg_t *pwm_cfg, unsigned int duty, size_t sample_int){
	static char line[LINE_LEN];
	static unsigned int h = 0;
	static char *pwm_mode[] = {
		"invalid",
		"fast",
		"phase correct"
	};


	line[h++] = c;

	printf(
		CLEARLINE "duty cycle:    %-5u\n"
		CLEARLINE "prescaler:     %-5u\n"
		CLEARLINE "mode:          %s\n"
		CLEARLINE "interval [ms]: %-5u\n"
		,
		(unsigned int)duty,
		(unsigned int)(pwm_cfg->prescaler == 0 ? 0 : (0x1 << (pwm_cfg->prescaler - 1))),
		pwm_mode[pwm_cfg->mode],
		(unsigned int)sample_int
	);

	fflush(stdout);

	write(1, line + h, LINE_LEN - h);
	write(1, line, h);

	fputs("\033[4F", stdout);
	fflush(stdout);

	if(h == LINE_LEN)
		h = 0;
}
