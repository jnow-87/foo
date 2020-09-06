/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef EXPORT_H
#define EXPORT_H


#include <sys/vector.h>
#include <stdio.h>
#include <node.h>


/* prototypes */
int export_c_header(FILE *fp);
int export_devices_c(device_node_t *node, FILE *fp);
int export_memory_c(memory_node_t *node, FILE *fp);

int export_header_header(FILE *fp);
int export_devices_header(device_node_t *node, FILE *fp);
int export_memory_header(memory_node_t *node, FILE *fp);

int export_make_header(FILE *fp);
int export_devices_make(device_node_t *node, FILE *fp);
int export_memory_make(memory_node_t *node, FILE *fp);


#endif // EXPORT_H
