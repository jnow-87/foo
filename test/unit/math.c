/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <sys/compiler.h>
#include <sys/limits.h>
#include <sys/math.h>
#include <test/test.h>


/* local/static prototypes */
static int test_align(unsigned int x, unsigned int base);
static int test_primes(size_t *nums, size_t n, bool expect);


/* local functions */
TEST(alignp2){
	int r = 0;


	r |= test_align(1, 1);
	r |= test_align(1, 2);
	r |= test_align(2, 2);
	r |= test_align(3, 2);
	r |= test_align(1, 4);
	r |= test_align(20, 16);

	return -r;
}

TEST(isprime){
	int r = 0;
	size_t primes[] = { 2, 3, 5, 19, 97, 599 },
		   non_primes[] = { 0, 1, 9, 10, 25, 49, 30031 };


	r |= test_primes(primes, sizeof_array(primes), true);
	r |= test_primes(non_primes, sizeof_array(non_primes), false);


	return -r;
}

TEST(prime_next){
	int r = 0;
	size_t primes[] = { 2, 3, 5, 7, 11, 13, 17, 19 };


	for(size_t i=0; i<sizeof_array(primes)-1; i++)
		r |= TEST_INT_EQ(prime_next(primes[i]), primes[i + 1]);

	r |= TEST_INT_EQ(prime_next(1), 2);
	r |= TEST_INT_EQ(prime_next(SIZE_MAX), 2);

	return r;
}

static int test_align(unsigned int x, unsigned int base){
	x = ALIGNP2(x, base);

	return TEST_INT_EQ(x % base, 0);
}

static int test_primes(size_t *nums, size_t n, bool expect){
	int r = 0;


	for(size_t i=0; i<n; i++){
		TEST_LOG("test: %zu to be %s prime\n", nums[i], expect ? "a" : "no");
		r |= TEST_INT_EQ(isprime(nums[i]), expect);
	}

	return r;
}
