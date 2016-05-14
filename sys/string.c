#include <sys/string.h>


/* global functions */
size_t strlen(char const *s){
	size_t i;

	
	i = 0;
	while(s[i]) i++;

	return i;
}

int strcmp(char const *s0, char const *s1){
	while(*s0 && *s1){
		if(*s0 < *s1)		return -1;
		else if(*s0 > *s1)	return 1;

		s0++;
		s1++;
	}

	if(*s0 || *s1)
		return -1;

	return 0;
}

void *memcpy(void *dest, void const *src, size_t n){
	size_t i;


	i = n;

	/* TODO optimise */
	if(src < dest && src + n > dest){
		dest += n - 1;
		src += n - 1;

		for(; i>0; i--)
			*(char*)dest-- = *(char*)src--;
	}
	else{
		for(; i>0; i--)
			*(char*)dest++ = *(char*)src++;

		dest -= n;
	}

	return dest;
}

void *memset(void *p, char c, size_t n){
	while(1){
		((char*)p)[--n] = c;

		if(n == 0)
			break;
	}

	return p;
}

int memcmp(void const *s0, void const *s1, size_t n){
	size_t i;


	for(i=0; i<n; i++){
		if(((char*)s0)[i] < ((char*)s1)[i])			return -1;
		else if(((char*)s0)[i] > ((char*)s1)[i])	return 1;
	}

	return 0;
}
