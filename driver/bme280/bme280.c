/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v1.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <kernel/devfs.h>
#include <driver/i2c.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/math.h>
#include <sys/string.h>
#include <sys/ioctl.h>
#include <sys/bme280.h>
#include <sys/sensor.h>


/* macros */
// registers
#define REGMAP_BASE	0x88
#define REGMAP_END	0xfe

#define CALIB0		REGMAP_BASE
#define ID			0xd0
#define RESET		0xe0
#define CALIB1		0xe1
#define CTRLHUM		0xf2
#define STATUS		0xf3
#define CTRLMEAS	0xf4
#define CFG			0xf5
#define SENSOR		0xf7

// bits
#define CTRLHUM_OSRS_H	0

#define STATUS_UPDATE	0
#define STATUS_MEASURE	3

#define CTRLMEAS_MODE	0
#define CTRLMEAS_OSRS_P	2
#define CTRLMEAS_OSRS_T	5

#define CFG_SPI_EN		0
#define CFG_FILTER		2
#define CFG_T_SB		5

// data conversion
#define CALIB16(msb, lsb)			((int16_t)((((int16_t)msb) << 8) | (lsb)))
#define CALIB16U(msb, lsb)			((uint16_t)((((uint16_t)msb) << 8) | (lsb)))
#define CALIB16H(msb, lsb)			((uint16_t)((((uint16_t)msb) << 4) | (lsb)))

#define SENSOR20(msb, lsb, xlsb)	((int32_t)((((int32_t)msb) << 12) | (((int32_t)lsb) << 4) | ((xlsb) >> 4)))
#define SENSOR16(msb, lsb)			((int32_t)(((int32_t)msb) << 8) | (lsb))


/* types */
typedef struct{
	uint16_t t1,
			 p1;
	int16_t t2,
			t3,
			p2,
			p3,
			p4,
			p5,
			p6,
			p7,
			p8,
			p9,
			h2,
			h4,
			h5;
	uint8_t h1,
			h3;
	int8_t h6;
} calib_t;

typedef struct{
	uint8_t addr;

	bme280_cfg_t config;
} dt_data_t;

typedef struct{
	i2c_t *i2c;
	dt_data_t *dtd;

	calib_t calib;
} dev_data_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *data, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);

static int configure(dev_data_t *bme, bme280_cfg_t *cfg);

static int read_calib(dev_data_t *bme);
static int read_sensor(dev_data_t *bme, envsensor_t *v);

static int32_t convert_temp(int32_t t_raw, calib_t *calib, int32_t *t_fine);
static uint32_t convert_press(int32_t p_raw, calib_t *calib, int32_t t_fine);
static uint8_t convert_hum(int32_t h_raw, calib_t *calib, int32_t t_fine);

static int wait_update(dev_data_t *bme);

static int reg_r(dev_data_t *bme, uint8_t addr, uint8_t *v, size_t n);
static int reg_w(dev_data_t *bme, uint8_t addr, uint8_t val);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *bme;
	devfs_ops_t ops;


	bme = kmalloc(sizeof(dev_data_t));

	if(bme == 0x0)
		goto err_0;

	bme->i2c = (i2c_t*)dt_itf;
	bme->dtd = (dt_data_t*)dt_data;

	if(configure(bme, &bme->dtd->config) != 0)
		goto err_1;

	if(read_calib(bme) != 0)
		goto err_1;

	memset(&ops, 0x0, sizeof(devfs_ops_t));

	ops.read = read;
	ops.ioctl = ioctl;

	if(devfs_dev_register(name, &ops, bme) == 0x0)
		goto err_1;

	return 0x0;


err_1:
	kfree(bme);

err_0:
	return 0x0;
}

driver_probe("bme280", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *data, size_t n){
	envsensor_t v;


	if(read_sensor(dev->payload, &v) != 0)
		return 0;

	n = MIN(n, sizeof(envsensor_t));
	memcpy(data, &v, n);

	return n;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	dev_data_t *bme = (dev_data_t*)dev->payload;


	if(n != sizeof(bme280_cfg_t))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(arg, &bme->dtd->config, sizeof(bme280_cfg_t));
		return 0;

	case IOCTL_CFGWR:
		if(configure(dev->payload, arg) != 0)
			return -1;

		memcpy(&bme->dtd->config, arg, sizeof(bme280_cfg_t));

		return 0;

	default:
		return_errno(E_NOSUP);
	}
}

static int configure(dev_data_t *bme, bme280_cfg_t *cfg){
	int r = 0;
	uint8_t id;


	/* reset */
	if(reg_w(bme, RESET, 0xb6) != 0)
		return -errno;

	r |= wait_update(bme);
	r |= reg_r(bme, ID, &id, 1);

	if(r != 0 || id != 0x60)
		return_errno(E_INVAL);

	/* configure */
	r |= reg_w(bme, CTRLHUM, cfg->oversampling_hum << CTRLHUM_OSRS_H);
	r |= reg_w(bme, CTRLMEAS,
		  (cfg->oversampling_temp << CTRLMEAS_OSRS_T)
		| (cfg->oversampling_press << CTRLMEAS_OSRS_P)
		| (cfg->mode << CTRLMEAS_MODE)
	);
	r |= reg_w(bme, CFG,
		  (cfg->standby_time_us << CFG_T_SB)
		| (cfg->filter << CFG_FILTER)
	);

	return r;
}

static int read_calib(dev_data_t *bme){
	calib_t *calib = &bme->calib;
	uint8_t data[CTRLHUM - CALIB0];


	if(reg_r(bme, CALIB0, data, sizeof(data)) != 0)
		return -1;

	calib->t1 = CALIB16U(data[1], data[0]);
	calib->t2 = CALIB16(data[3], data[2]);
	calib->t3 = CALIB16(data[5], data[4]);

	calib->p1 = CALIB16U(data[7], data[6]);
	calib->p2 = CALIB16(data[9], data[8]);
	calib->p3 = CALIB16(data[11], data[10]);
	calib->p4 = CALIB16(data[13], data[12]);
	calib->p5 = CALIB16(data[15], data[14]);
	calib->p6 = CALIB16(data[17], data[16]);
	calib->p7 = CALIB16(data[19], data[18]);
	calib->p8 = CALIB16(data[21], data[20]);
	calib->p9 = CALIB16(data[23], data[22]);

	calib->h1 = data[25];
	calib->h2 = CALIB16(data[90], data[89]);
	calib->h3 = data[91];
	calib->h4 = CALIB16H(data[92], (data[93] & 0x0f));
	calib->h5 = CALIB16H(data[94], (data[93] >> 4));
	calib->h6 = data[95];

	return 0;
}

static int read_sensor(dev_data_t *bme, envsensor_t *v){
	bme280_cfg_t *cfg = &bme->dtd->config;
	uint8_t data[REGMAP_END - SENSOR + 1];
	int32_t t,
			t_fine,
			p,
			h;


	/* trigger measurement if in forced mode */
	if(cfg->mode == BME_MODE_FORCED){
		data[0] = (cfg->oversampling_temp << CTRLMEAS_OSRS_T)
				| (cfg->oversampling_press << CTRLMEAS_OSRS_P)
				| (cfg->mode << CTRLMEAS_MODE)
				;

		if(reg_w(bme, CTRLMEAS, data[0]) != 0)
			return -1;
	}

	/* wait for measurement and update to complete */
	wait_update(bme);

	/* read and process sensor data */
	if(reg_r(bme, SENSOR, data, sizeof(data)) != 0)
		return -1;

	p = SENSOR20(data[0], data[1], data[2]);
	t = SENSOR20(data[3], data[4], data[5]);
	h = SENSOR16(data[6], data[7]);

	t = convert_temp(t, &bme->calib, &t_fine);

	if(bme->dtd->config.features & BME_FTR_PRESS)
		v->press.pa = convert_press(p, &bme->calib, t_fine);

	if(bme->dtd->config.features & BME_FTR_HUM)
		v->hum.perc = convert_hum(h, &bme->calib, t_fine);

	t_fine = t / 100;
	v->temp.mdegc = (t - t_fine * 100) * 10;
	v->temp.degc = MAX(t_fine, -32768);
	v->temp.degc = MIN(v->temp.degc, 32767);

	return 0;
}

/**
 * \brief	Compensation function taken from the bme280 manual.
 *
 * 			Converts the sensor reading to a temperature in deg Celsius with a
 * 			resolution of 0.01, i.e. an value of 5123 equals 51.23 deg Celsius.
 */
static int32_t convert_temp(int32_t t_raw, calib_t *calib, int32_t *t_fine){
	int32_t t;


	t = (((t_raw >> 3) - ((int32_t)calib->t1 << 1)) * (int32_t)calib->t2) >> 11;
	t += ((SQUARE((t_raw >> 4) - (int32_t)calib->t1) >> 12) * (int32_t)calib->t3) >> 14;
	*t_fine = t;

	return (t * 5 + 128) >> 8;
}

/**
 * \brief	Compensation function taken from the bme280 manual.
 *
 * 			Converts the sensor reading to a pressure value in Pa.
 */
static uint32_t convert_press(int32_t p_raw, calib_t *calib, int32_t t_fine){
	int32_t v0,
			v1;
	uint32_t p;


	v0 = ((int32_t)t_fine >> 1) - 64000;
	v1 = (SQUARE(v0 >> 2) >> 11) * (int32_t)calib->p6;
	v1 += (v0 * (int32_t)calib->p5) << 1;
	v1 = (v1 >> 2) + ((int32_t)calib->p4 << 16);
	v0 = (((calib->p3 * (SQUARE(v0 >> 2) >> 13)) >> 3) + (((int32_t)calib->p2 * v0) >> 1)) >> 18;
	v0 = ((32768 + v0) * (int32_t)calib->p1) >> 15;

	if(v0 == 0)
		return 0;

	p = ((uint32_t)((int32_t)1048576 - p_raw) - (v1 >> 12)) * 3125;
	p = (p < 0x80000000) ? ((p << 1) / (uint32_t)v0) : ((p / (uint32_t)v0) << 1);

	v0 = (((int32_t)calib->p9) * (int32_t)(SQUARE(p >> 3) >> 13)) >> 12;
	v1 = ((int32_t)(p >> 2) * (int32_t)calib->p8) >> 13;

	return (int32_t)p + ((v0 + v1 + calib->p7) >> 4);
}

/**
 * \brief	Compensation function taken from the bme280 manual.
 *
 * 			Converts the sensor reading to a humidity value in %RH.
 */
static uint8_t convert_hum(int32_t h_raw, calib_t *calib, int32_t t_fine){
	int32_t h,
			v0,
			v1;


	v0 = t_fine - 76800;
	v1 = ((v0 * (int32_t)calib->h6) >> 10) * (((v0 * (int32_t)calib->h3) >> 11) + ((int32_t)32768));

	h = (((h_raw << 14) - ((int32_t)calib->h4 << 20) - ((int32_t)calib->h5 * v0)) + ((int32_t)16384)) >> 15;
	h *= (((v1 >> 10) + ((int32_t)2097152)) * (int32_t)calib->h2 + 8192) >> 14;
	h -= ((SQUARE(h >> 15) >> 7) * (int32_t)calib->h1) >> 4;
	h = MAX(h, 0) >> 22;

	return MIN(h, 100);
}

static int wait_update(dev_data_t *bme){
	uint8_t status = 0xff;


	while(status & ((0x1 << STATUS_MEASURE) | (0x1 << STATUS_UPDATE))){
		if(reg_r(bme, STATUS, &status, 1) != 0)
			return -1;
	}

	return 0;
}

static int reg_r(dev_data_t *bme, uint8_t addr, uint8_t *v, size_t n){
	if(i2c_write(bme->i2c, bme->dtd->addr, &addr, 1) != 0)
		return -1;

	return i2c_read(bme->i2c, bme->dtd->addr, v, n);
}

static int reg_w(dev_data_t *bme, uint8_t addr, uint8_t val){
	uint8_t data[] = { addr, val };


	return i2c_write(bme->i2c, bme->dtd->addr, data, 2);
}
