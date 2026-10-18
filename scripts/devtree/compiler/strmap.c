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
static map_t strmap = MAP_INITIALISER(hash, kmatch, kvalloc, kvfree);


/* global functions */
void strmap_free(void){
	map_free(&strmap);
}

char const *strmap_add(char const *s){
	if(map_add(&strmap, (void*)s, "") < 0){
		devtree_parser_error("out of memory");

		return 0x0;
	}

	return map_lookup(&strmap, (void*)s);
}


/* local functions */
static size_t hash(void *key){
	size_t v = 0;


	for(char *s=key; *s!=0; s++)
		v += *s;

	return v;
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
