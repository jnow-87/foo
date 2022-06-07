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
#include <kernel/devfs.h>
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

	gpio_type_t pin_mask,
				in_mask,
				out_mask,
				int_mask,
				invert_mask;	/**< 0 = pass through, 1 = invert */

	uint8_t int_num;			/**< cf. int_num_t */
} gpio_cfg_t;

typedef struct{
	gpio_type_t (*read)(void *hw);
	int (*write)(gpio_type_t v, void *hw);

	int (*configure)(gpio_cfg_t *cfg, void *hw);
} gpio_ops_t;

typedef struct gpio_siglst_t{
	struct gpio_siglst_t *prev,
						 *next;

	signal_t sig;
	thread_t *thread;
	fs_filed_t *fd;

	gpio_type_t mask;
} gpio_siglst_t;

typedef struct{
	gpio_ops_t ops;
	gpio_cfg_t *cfg;
	void *hw;

	gpio_siglst_t *sigs;
	mutex_t mtx;

	devfs_dev_t *dev;
} gpio_t;


/* prototypes */
gpio_t *gpio_create(char const *name, gpio_ops_t *ops, gpio_cfg_t *cfg, void *hw);
void gpio_destroy(gpio_t *gpio);


#endif // DRIVER_GPIO_H
