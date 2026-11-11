/**
 * Copyright (C) 2026 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <sys/map.h>
#include <sys/string.h>
#include <parser.tab.h>


/* local/static prototypes */
static size_t hash(void *key);
static int kmatch(void *key0, void *key1);
static int kvalloc(void **key, void **value);
static void kvfree(void *key, void *value);


/* static variables */
static map_t str_cache = MAP_INITIALISER(hash, kmatch, kvalloc, kvfree);


/* global functions */
void str_cache_reset(void){
	map_reset(&str_cache);
}

char const *str_cache_add(char const *s){
	s = map_add(&str_cache, (void*)s, (void*)s);

	if(s == 0x0)
		devtree_parser_error("out of memory");

	return s;
}


/* local functions */
static size_t hash(void *key){
	size_t h = 0;


	for(char *s=key; *s!=0; s++)
		h += *s;

	return h;
}

static int kmatch(void *key0, void *key1){
	return strcmp(key0, key1);
}

static int kvalloc(void **key, void **val){
	*key = strdup(*key);
	*val = *key;

	return (*key != 0x0) ? 0 : -1;
}

static void kvfree(void *key, void *val){
	free(key);
}
