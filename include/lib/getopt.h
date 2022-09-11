/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#ifndef LIB_GETOPT_H
#define LIB_GETOPT_H


/* external variables */
extern char optopt;
extern int opterr;
extern int optind;
extern char *optarg;


/* prototypes */
void getopt_reset(void);
char getopt(int argc, char **argv, char const *optstr);


#endif // LIB_GETOPT_H
