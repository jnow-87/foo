/**
 * Copyright (C) 2015 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Most of source code is taken from the linux kernel's build helper fixdep.c
 * but has been restructured for the purpose of this project.
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "hashtbl.h"


/* macros */
#define HASHSZ 256


/* static prototypes */
static unsigned int strhash(char const *str, unsigned int sz);


/* global variables */
static struct item *hashtab[HASHSZ];


/* global functions */
/*
 * Lookup a value in the configuration string.
 */
int hashtbl_lookup(char const *name, int n, unsigned int hash){
	for(struct item *aux=hashtab[hash % HASHSZ]; aux; aux=aux->next){
		if(aux->hash == hash && aux->len == n &&
		    memcmp(aux->name, name, n) == 0)
			return 1;
	}

	return 0;
}

/*
 * Add a new value to the configuration string.
 */
int hashtbl_add(char const *name, int n){
	struct item *aux;
	unsigned int hash = strhash(name, n);


	if(hashtbl_lookup(name, n, hash))
	    return 1;

	aux = malloc(sizeof(*aux) + n);

	if(!aux){
		perror("fixdep:malloc");
		exit(1);
	}

	memcpy(aux->name, name, n);
	aux->len = n;
	aux->hash = hash;
	aux->next = hashtab[hash % HASHSZ];
	hashtab[hash % HASHSZ] = aux;

	return 0;
}

/*
 * Clear the set of configuration strings.
 */
void hashtbl_clear(void){
	struct item *next;


	for(size_t i=0; i<HASHSZ; i++){
		for(struct item *aux=hashtab[i]; aux; aux=next){
			next = aux->next;
			free(aux);
		}

		hashtab[i] = NULL;
	}
}


/* local functions */
unsigned int strhash(char const *str, unsigned int sz){
	/* fnv32 hash */
	unsigned int hash = 2166136261U;


	for(size_t i=0; i<sz; i++)
		hash =(hash ^ str[i]) * 0x01000193;
	return hash;
}
