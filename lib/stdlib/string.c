/* global functions */
int atoi(char const *s){
	int r,
		x;


	r = 0;

	for(;*s!=0; s++){
		x = *s - '0';

		if(x < 0 || x > 9)
			return 0;

		r = r * 10 + x;
	}

	return r;
}
