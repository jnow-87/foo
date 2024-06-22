/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <unistd.h>
#include <sys/compiler.h>
#include <sys/string.h>
#include <sys/sensor.h>
#include <shell/shell.h>
#include <shell/cmd.h>


/* macros */
#define ARGS \
	"<device> <type>\n" \
	"\n" \
	"types:\n" \
	"    0: temperature\n" \
	"    1: pressure\n" \
	"    2: humidity\n" \
	"    3: environment"

#define OPTS	""


/* types */
typedef struct{
	void (*fmt)(void *data);
	size_t data_offset;
} sensor_t;


/* local/static prototypes */
static void fmt_temp(void *data);
static void fmt_press(void *data);
static void fmt_hum(void *data);
static void fmt_env(void *data);


/* local functions */
static int exec(int argc, char **argv){
	sensor_t sensors[] = {
		{ .fmt = fmt_temp,	.data_offset = offsetof(envsensor_t, temp) },
		{ .fmt = fmt_press,	.data_offset = offsetof(envsensor_t, press) },
		{ .fmt = fmt_hum,	.data_offset = offsetof(envsensor_t, hum) },
		{ .fmt = fmt_env,	.data_offset = 0 },
	};
	int r = 0;
	int fd;
	unsigned int type;
	uint8_t data[sizeof(envsensor_t)];


	if(argc < 3)
		return CMD_HELP(argv[0], 0x0);

	type = atoi(argv[2]);

	if(type >= sizeof_array(sensors))
		return -ERROR("invalid sensor type %u", type);

	fd = open(argv[1], O_RDONLY);

	if(fd < 0)
		return -ERROR("opening %s", argv[1]);

	r = (read(fd, &data, sizeof(envsensor_t)) != sizeof(envsensor_t));
	close(fd);

	if(r != 0)
		return -ERROR("read");

	sensors[type].fmt(data + sensors[type].data_offset);

	return 0;
}

command("sensorcat", exec);

static void fmt_temp(void *data){
	temperature_t *temp = (temperature_t*)data;


	printf("temperature [degC]: %d.%03u\n", temp->degc, temp->mdegc);
}

static void fmt_press(void *data){
	printf("pressure [Pa]: %u\n", ((pressure_t*)data)->pa);
}

static void fmt_hum(void *data){
	printf("humidity [%]: %u\n", ((humidity_t*)data)->perc);
}

static void fmt_env(void *data){
	envsensor_t *env = (envsensor_t*)data;


	fmt_temp(&env->temp);
	fmt_hum(&env->hum);
	fmt_press(&env->press);
}
