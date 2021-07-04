/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <driver/term.h>
#include <sys/types.h>


/* types */
typedef char *(*flag_hdlr_t)(char *s, size_t idx, size_t n, term_t *term);


/* local/static prototypes */
static char *i_crnl(char *s, size_t idx, size_t n, term_t *term);
static char *i_nlcr(char *s, size_t idx, size_t n, term_t *term);

static char *o_crnl(char *s, size_t idx, size_t n, term_t *term);
static char *o_nlcr(char *s, size_t idx, size_t n, term_t *term);

static char *l_echo(char *s, size_t idx, size_t n, term_t *term);


/* static variables */
static flag_hdlr_t iflag_hdlr[] = {
		i_crnl,
		i_nlcr,
		0x0
};

static flag_hdlr_t oflag_hdlr[] = {
		o_crnl,
		o_nlcr,
		0x0
};

static flag_hdlr_t lflag_hdlr[] = {
		l_echo,
		0x0
};

static flag_hdlr_t *flag_hdlr[] = {
	iflag_hdlr,
	oflag_hdlr,
	lflag_hdlr,
};


/* global functions */
char *term_flags_apply(term_t *term, char *s, size_t n, size_t incr, term_flag_type_t fl_type, uint8_t flags){
	size_t i,
		   j,
		   n_put;
	char *x;


	x = s;

	for(i=0; i<n; i+=incr){
		for(j=0; flag_hdlr[fl_type][j]!=0x0; j++){
			if((flags & (0x1 << j)) == 0)
				continue;

			n_put = x - s;
			x = flag_hdlr[fl_type][j](x, i - n_put, n - n_put, term);

			if(x == 0x0)
				return 0x0;
		}
	}

	return x;
}


/* local functions */
static char *i_crnl(char *s, size_t idx, size_t n, term_t *term){
	if(s[idx] == '\r')
		s[idx] = '\n';

	return s;
}

static char *i_nlcr(char *s, size_t idx, size_t n, term_t *term){
	if(s[idx] == '\n')
		s[idx] = '\r';

	return s;
}

static char *o_crnl(char *s, size_t idx, size_t n, term_t *term){
	if(s[idx] == '\r')
		s[idx] = '\n';

	return s;
}

static char *o_nlcr(char *s, size_t idx, size_t n, term_t *term){
	size_t r;


	if(s[idx] != '\n')
		return s;

	r = term_puts(term, s, idx);
	r += term_puts(term, "\r\n", 2);

	return (r == idx + 2) ? s + idx + 1 : 0x0;
}

static char *l_echo(char *s, size_t idx, size_t n, term_t *term){
	return (term_puts(term, s, n) == n) ? s : 0x0;
}
