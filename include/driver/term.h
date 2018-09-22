#ifndef DRIVER_TERM_H
#define DRIVER_TERM_H


#include <config/config.h>
#include <kernel/signal.h>
#include <kernel/devfs.h>
#include <sys/ringbuf.h>
#include <sys/term.h>


/* incomplete types */
struct term_t;


/* types */
typedef struct{
	int (*config)(struct term_t *term, term_cfg_t *cfg);
	int (*puts)(struct term_t *term, char const *s, size_t n);
} term_ops_t;

typedef struct term_t{
	term_cfg_t cfg;
	term_err_t rx_err;

	ringbuf_t rx_buf;
	ksignal_t *rx_rdy;

	void *data;
	term_ops_t ops;
	devfs_dev_t *dev;
} term_t;


/* prototypes */
term_t *term_register(char const *suffix, term_ops_t *ops, void *data);
void term_release(term_t *term);


#endif // DRIVER_TERM_H
