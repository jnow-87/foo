#include <sys/stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmd/cmd.h>


/* local/static prototypes */
static int help(char const *prog_name, char const *msg, ...);


/* local functions */
static int exec(int argc, char **argv){
	int i;
	uint8_t x;
	bool binary;


	binary = false;

	/* check options */
	for(i=1; i<argc && argv[i][0]=='-'; i++){
		switch(argv[i][1]){
		case 'b':
			binary = true;
			break;

		default:
			return help(argv[0], "invalid option '%s'\n", argv[i]);
		}
	}

	/* echo non-option arguments */
	for(; i<argc; i++){
		if(binary){
			x = strtol(argv[i], 0x0, 0);
			fwrite(&x, sizeof(x), stdout);
		}
		else{
			fputs(argv[i], stdout);
			fputc(' ', stdout);
		}
	}

	if(!binary)
		fputc('\n', stdout);

	return 0;
}

command("echo", exec);

static int help(char const *prog_name, char const *msg, ...){
	va_list lst;


	if(msg){
		va_start(lst, msg);
		vfprintf(stderr, msg, lst);
		va_end(lst);
		fputs("\n", stderr);
	}

	fprintf(stderr,
		"usage: %s <options> <args>\n"
		"\noptions:\n"
		"%15.15s    %s\n"
		, prog_name
		, "-b", "convert <args> to binary representation"
	);

	return (msg ? 1 : 0);
}
