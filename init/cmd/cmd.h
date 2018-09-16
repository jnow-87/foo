#ifndef INIT_CMD_H
#define INIT_CMD_H


#include <config/config.h>


/* types */
typedef struct cmd_t{
	char const *name;
	int (*exec)(int argc, char **argv);
} cmd_t;


/* prototypes */
#ifdef CONFIG_INIT_HELP
int help(int argc, char **argv);
#else
#define help	0x0
#endif // CONFIG_INIT_HELP

#ifdef CONFIG_INIT_CD
int cd(int argc, char **argv);
#else
#define cd		0x0
#endif // CONFIG_INIT_CD

#ifdef CONFIG_INIT_LS
int ls(int argc, char **argv);
#else
#define ls		0x0
#endif // CONFIG_INIT_LS

#ifdef CONFIG_INIT_ECHO
int echo(int argc, char **argv);
#else
#define echo	0x0
#endif // CONFIG_INIT_ECHO

#ifdef CONFIG_INIT_CAT
int cat(int argc, char **argv);
#else
#define cat		0x0
#endif // CONFIG_INIT_CAT

#ifdef CONFIG_INIT_TEST
int test(int argc, char **argv);
#else
#define test	0x0
#endif // CONFIG_INIT_TEST


#endif // INIT_CMD_H
