/**
 * Copyright (C) 2026 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/math.h>
#include <sys/types.h>


/* global functions */
size_t prime_next(size_t n){
	// check for even primes and overflow
	if(n < 2 || n + 1 < 2)
		return 2;

	// ensure n is an odd number or, if it already is, skip
	// ahead to the next one in case n already is a prime
	n += (n % 2 == 0) ? 1 : 2;

	while(!isprime(n))
		n += 2;

	return n;
}

bool isprime(size_t n){
	if(n < 2)
		return false;

	if(n < 4)
		return true;

	// checks to allow skipping ahead by 6 in next loop
	if(n % 2 == 0 || n % 3 == 0)
		return false;

	for(size_t i=5; SQUARE(i)<=n; i+=6){
		if(n % i == 0 || n % (i + 2) == 0)
			return false;
	}

	return true;
}
