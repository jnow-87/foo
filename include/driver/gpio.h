/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H


#include <kernel/thread.h>
#include <kernel/fs.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/signal.h>
#include <sys/gpio.h>


/* types */
typedef enum{
	GM_NORMAL = 1,
	GM_STRICT,
} gpio_mode_t;

typedef struct{
	/**
	 * NOTE fixed-size types are used to allow
	 * 		using this type with the device tree
	 */

	uint8_t mode;				/**< cf. gpio_mode_t */

	intgpio_t pin_mask,
			  in_mask,
			  out_mask,
			  int_mask,
			  invert_mask;		/**< 0 = pass through, 1 = invert */

	uint8_t int_num;			/**< cf. int_num_t */
} gpio_cfg_t;

typedef struct{
	int (*configure)(gpio_cfg_t *cfg, void *hw);
	intgpio_t (*read)(void *hw);
	int (*write)(intgpio_t v, void *hw);

	void *hw;
} gpio_itf_t;

typedef struct gpio_siglst_t{
	struct gpio_siglst_t *prev,
						 *next;

	signal_t signum;
	thread_t *thread;
	fs_filed_t *fd;

	intgpio_t mask;
} gpio_siglst_t;

typedef struct{
	gpio_itf_t *itf;
	gpio_cfg_t *cfg;

	gpio_siglst_t *sigs;
	mutex_t mtx;

	intgpio_t int_state;
} gpio_t;


/* prototypes */
gpio_t *gpio_create(gpio_itf_t *itf, gpio_cfg_t *cfg);
void gpio_destroy(gpio_t *gpio);

int gpio_configure(gpio_t *gpio);
intgpio_t gpio_read(gpio_t *gpio);
int gpio_write(gpio_t *gpio, intgpio_t v);

int gpio_int_register(gpio_t *gpio, fs_filed_t *fd, gpio_int_cfg_t *cfg);
int gpio_int_release(gpio_t *gpio, fs_filed_t *fd);
void gpio_int_probe(gpio_t *gpio, fs_filed_t *fd, gpio_int_cfg_t *cfg);


#endif // DRIVER_GPIO_H
