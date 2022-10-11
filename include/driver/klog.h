/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_KLOG_H
#define DRIVER_KLOG_H


typedef struct{
	size_t (*puts)(char const *s, size_t n, void *hw);

	void *hw;
} klog_itf_t;


#endif // DRIVER_KLOG_H
