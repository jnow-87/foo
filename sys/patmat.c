/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/memory.h>
#include <sys/string.h>
#include <sys/patmat.h>


/* types */
typedef struct{
	char specifier;
	size_t size;
	bool (*match)(char c, patmat_spec_t *spec);
} spec_cfg_t;


/* local/static prototypes */
static int pattern_init(patmat_pattern_t *pat, char const *text);
static void pattern_destroy(patmat_pattern_t *pat);
static void pattern_reset(patmat_pattern_t *pat);

static patmat_state_t pattern_match(patmat_pattern_t *pat, char c);

static bool match_int(char c, patmat_spec_t *spec);
static bool match_char(char c, patmat_spec_t *spec);
static bool match_string(char c, patmat_spec_t *spec);

static patmat_spec_t *spec_alloc(char const *spec);
static bool spec_completed(patmat_spec_t *spec, char const *pattern);


/* static variables */
static spec_cfg_t matcher[] = {
	{ .specifier = 'd',	.size = sizeof(int),	.match = match_int },		// PMS_INT
	{ .specifier = 'c',	.size = sizeof(char),	.match = match_char },		// PMS_CHAR
	{ .specifier = 's',	.size = 0,				.match = match_string },	// PMS_STR
};

STATIC_ASSERT(sizeof_array(matcher) == PMS_MAX);


/* global functions */
patmat_t *patmat_init(char const **patterns, ssize_t npat){
	patmat_t *pm;


	pm = sys_calloc(1, sizeof(patmat_t) + sizeof(patmat_pattern_t) * npat);

	if(pm == 0x0)
		goto err_0;

	for(ssize_t i=0; i<npat; i++){
		if(pattern_init(pm->patterns + i, patterns[i]) != 0)
			goto err_1;
	}

	pm->npat = npat;
	pm->last_match = -1;

	return pm;


err_1:
	patmat_destroy(pm);

err_0:
	return 0x0;
}

void patmat_destroy(patmat_t *pm){
	for(ssize_t i=0; i<pm->npat; i++)
		pattern_destroy(pm->patterns + i);

	sys_free(pm);
}

void patmat_reset(patmat_t *pm){
	for(ssize_t i=0; i<pm->npat; i++)
		pattern_reset(pm->patterns + i);

	pm->last_match = -1;
}

ssize_t patmat_get_results(patmat_t *pm, void **results){
	patmat_pattern_t *pat;


	if(pm->last_match < 0)
		return -1;

	pat = pm->patterns + pm->last_match;

	for(size_t i=0; i<pat->nspec; i++)
		results[i] = pat->specs[i]->payload;

	return pm->last_match;
}

/**
 * \brief
 *
 * \return	PM_NOMATCH		no match possible
 * 			PM_MATCHABLE	match still possible
 * 			else			the index of the pattern that matched
 */
ssize_t patmat_match_char(patmat_t *pm, char c){
	size_t matchable = 0;
	patmat_pattern_t *pat;


	if(c <= 0)
		return PM_NOMATCH;

	for(ssize_t i=0; i<pm->npat; i++){
		pat = pm->patterns + i;

		if(pat->state != PM_MATCHABLE)
			continue;

		pat->state = pattern_match(pat, c);

		if(pat->state == PM_MATCH){
			pm->last_match = i;
			return i;
		}

		if(pat->state == PM_MATCHABLE)
			matchable++;
	}

	return (matchable == 0) ? PM_NOMATCH : PM_MATCHABLE;
}

/**
 * \brief
 *
 * \return	PM_NOMATCH		no match possible
 * 			else			the index of the pattern that matched
 */
ssize_t patmat_match_string(patmat_t *pm, char const *s){
	patmat_state_t r;


	patmat_reset(pm);

	while(*s){
		r = patmat_match_char(pm, *(s++));

		if(r != PM_MATCHABLE)
			return r;
	}

	return PM_NOMATCH;
}


/* local functions */
/**
 * \pre		pat is zero initialised
 */
static int pattern_init(patmat_pattern_t *pat, char const *text){
	size_t spec = 0;


	/* allocate pattern */
	pat->text = sys_malloc(strlen(text) + 1);

	if(pat->text == 0x0)
		goto err_0;

	strcpy(pat->text, text);

	/* allocate specifier array */
	pat->nspec = strcnt(text, '%');
	pat->specs = sys_calloc(pat->nspec, sizeof(patmat_spec_t*));

	if(pat->nspec != 0 && pat->specs == 0x0)
		goto err_1;

	/* allocate individual specifiers */
	for(size_t i=0; text[i]!=0; i++){
		if(text[i] == '%'){
			pat->specs[spec] = spec_alloc(text + i + 1);

			if(pat->specs[spec] == 0x0)
				goto err_1;

			// pattern is not allowed to end on a specifier
			if(text[i + pat->specs[spec]->len + 1] == 0)
				goto err_1;

			spec++;
		}
	}

	pattern_reset(pat);

	return 0;


err_1:
	pattern_destroy(pat);

err_0:
	return -1;
}

static void pattern_destroy(patmat_pattern_t *pat){
	for(size_t i=0; i<pat->nspec && pat->specs; i++)
		sys_free(pat->specs[i]);

	sys_free(pat->specs);
	sys_free(pat->text);
}

static void pattern_reset(patmat_pattern_t *pat){
	patmat_spec_t *spec;


	pat->state = PM_MATCHABLE;
	pat->pat_idx = 0;
	pat->spec_idx = 0;

	for(size_t i=0; i<pat->nspec; i++){
		spec = pat->specs[i];

		spec->chars_matched = 0;

		if(matcher[spec->specifier].size != 0)
			memset(spec->payload, 0, matcher[spec->specifier].size);
	}
}

static patmat_state_t pattern_match(patmat_pattern_t *pat, char c){
	char c_pat = pat->text[pat->pat_idx];
	patmat_spec_t *spec;


	/* literal match */
	if(c == c_pat){
		pat->pat_idx++;
		goto check_match;
	}

	/* neither literal nor specifier match */
	if(c_pat != '%')
		return PM_NOMATCH;

	/* check if end of current specifier is reached */
	spec = pat->specs[pat->spec_idx];

	if(pat->text[pat->pat_idx + spec->len + 1] == c){
		if(!spec_completed(spec, pat->text))
			return PM_NOMATCH;

		pat->pat_idx += spec->len + 2;
		pat->spec_idx++;

		goto check_match;
	}

	/* match specifier */
	if(!matcher[spec->specifier].match(c, spec)){
		pat->state = PM_NOMATCH;

		return PM_NOMATCH;
	}

check_match:
	return pat->text[pat->pat_idx] == 0 ? PM_MATCH : PM_MATCHABLE;
}

static bool match_int(char c, patmat_spec_t *spec){
	int num;


	if(c < '0' || c > '9')
		return false;

	memcpy(&num, spec->payload, sizeof(int));
	num = num * 10 + c - '0';
	memcpy(spec->payload, &num, sizeof(int));

	spec->chars_matched++;

	return true;
}

static bool match_char(char c, patmat_spec_t *spec){
	if(spec->chars_matched != 0)
		return false;

	spec->payload[0] = c;
	spec->chars_matched++;

	return true;
}

static bool match_string(char c, patmat_spec_t *spec){
	spec->payload[spec->chars_matched] = c;
	spec->chars_matched++;

	return true;
}

static patmat_spec_t *spec_alloc(char const *spec){
	spec_cfg_t *cfg = 0x0;
	size_t i,
		   size;
	char *end;
	patmat_spec_t *s;


	/* parse data size */
	size = strtol(spec, &end, 10);

	/* get specifier */
	for(i=0; i<sizeof_array(matcher); i++){
		if(matcher[i].specifier == *end){
			cfg = matcher + i;
			break;
		}
	}

	if(cfg == 0x0)
		return 0x0;

	/* set data size */
	if(size == 0)
		size = cfg->size;

	if(size == 0)
		return 0x0;

	/* allocate specifier */
	s = sys_malloc(sizeof(patmat_spec_t) + size);

	if(s == 0x0)
		return 0x0;

	s->specifier = i;
	s->len = end - spec + 1;
	s->chars_matched = 0;

	return s;
}

static bool spec_completed(patmat_spec_t *spec, char const *pattern){
	/* at least one character has to parsed */
	if(spec->chars_matched == 0)
		return false;

	/* terminate strings */
	if(spec->specifier == PMS_STR)
		spec->payload[spec->chars_matched] = 0;

	return true;
}
