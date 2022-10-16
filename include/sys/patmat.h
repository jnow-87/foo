/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef PATMAT_H
#define PATMAT_H


#include <sys/types.h>


/* macros */
#define PATMAT_RESULT_INT(results, idx)		(*((int*)((results)[idx])))
#define PATMAT_RESULT_CHAR(results, idx)	(*((char*)((results)[idx])))
#define PATMAT_RESULT_STR(results, idx)		((results)[idx])


/* types */
typedef enum{
	PM_MATCHABLE = -2,
	PM_NOMATCH = -1,
	PM_MATCH = 0,
} patmat_state_t;

typedef enum{
	PMS_INT = 0,
	PMS_CHAR,
	PMS_STR,
} patmat_spec_type_t;

typedef struct patmat_spec_t{
	patmat_spec_type_t specifier;
	size_t len;
	size_t chars_matched;

	uint8_t payload[];
} patmat_spec_t;

typedef struct{
	char *text;

	patmat_spec_t **specs;
	size_t nspec;

	patmat_state_t state;
	size_t pat_idx,
		   spec_idx;
} patmat_pattern_t;

typedef struct{
	ssize_t last_match;

	ssize_t npat;
	patmat_pattern_t patterns[];
} patmat_t;


/* prototypes */
patmat_t *patmat_init(char const **patterns, ssize_t npat);
void patmat_destroy(patmat_t *pm);
void patmat_reset(patmat_t *pm);

ssize_t patmat_get_results(patmat_t *pm, void **results);

ssize_t patmat_match_char(patmat_t *pm, char c);
ssize_t patmat_match_string(patmat_t *pm, char const *s);


#endif // PATMAT_H
