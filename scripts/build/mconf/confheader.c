#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lkc.h"


/* local prototypes */
static int conf_write_confheader(const char *path);
static int conf_write_autoconfig_dep(const char *path, char const *conf_header);


/* global functions */
int main(int argc, char **argv){
	size_t i;


	/* check args */
	if(argc < 4){
		printf("usage: %s <Kconfig> <conf-header path> <config.h name>\n", argv[0]);
		return 1;
	}

	i = strlen(argv[2]);

	if(argv[2][i - 1] == '/')
		argv[2][i - 1] = 0;

	/* read Kconfig file */
	conf_parse(argv[1]);

	/* read config file */
	if(conf_read(NULL)){
		fprintf(stderr, "error parsing config\n");
		return 1;
	}

	/* create config headers */
	if(conf_write_confheader(argv[2])){
		fprintf(stderr, "error creating config headers\n");
		return 1;
	}

	/* create configuration header dependency file */
	if(conf_write_autoconfig_dep(argv[2], argv[3])){
		fprintf(stderr, "error creating config header dependency file\n");
		return 1;
	}

	/* create configuration header */
	if(conf_write_autoconf(argv[3])){
		fprintf(stderr, "error writing config header\n");
		return 1;
	}

	return 0;
}


/* local functions */
int conf_write_confheader(const char *path){
	char fname[strlen(path) + PATH_MAX + 1],
		 b[PATH_MAX + 1],
		 c;
	char *s,
		 *p;
	struct symbol *sym;
	struct stat sb;
	int i,
		fd;
	unsigned int plen = strlen(path);


	strcpy(fname, path);
	strcpy(fname + plen, "/");
	plen++;

	for_all_symbols(i, sym){
		if(sym->name == 0 || sym->type == S_UNKNOWN || sym->type  == S_OTHER)
			continue;

		sym_calc_value(sym);

		// replace '_' by '/' and append ".h"
		s = sym->name;
		p = fname + plen;
		while((c = *s++)){
			c = tolower(c);
			*p++ = (c == '_') ? '/' : c;
		}

		strcpy(p, ".h");

		// try to open file
		fd = open(fname, O_RDONLY | O_CREAT, 0644);

		if(fd == -1){
			if(errno != ENOENT)
				goto err_0;

			// create directory components
			p = fname;

			while ((p = strchr(p, '/'))) {
				*p = 0;

				if(stat(fname, &sb) && mkdir(fname, 0755))
					goto err_0;

				*p++ = '/';
			}

			// retry opening header
			fd = open(fname, O_RDONLY | O_CREAT, 0644);

			if(fd == -1)
				goto err_0;
		}

		// read last value from file and compare against sym
		if(fstat(fd, &sb))
			goto err_1;

		if(sb.st_size > PATH_MAX){
			fprintf(stderr, "file \"%s\" too large for buffer\n", fname);
			goto err_1;
		}

		if(read(fd, b, sb.st_size) != sb.st_size)
			goto err_1;

		b[sb.st_size] = 0;

		if(strcmp(b, sym_get_string_value(sym)) != 0){
			// update config header if last and current value to not match
			close(fd);

			fprintf(stdout, "update config header %s\n", fname);
			fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			write(fd, sym_get_string_value(sym), strlen(sym_get_string_value(sym)));
		}

		close(fd);
	}

	return 0;

err_1:
	close(fd);

err_0:
	return -1;
}

static int conf_write_autoconfig_dep(const char *path, char const *conf_header){
	char depfile[strlen(conf_header) + 3],
		 c;
	char *s;
	struct symbol *sym;
	FILE *fp;
	int i;


	// depfile = conf_header.d
	strcpy(depfile, conf_header);
	strcpy(depfile + strlen(conf_header), ".d");

	fp = fopen(depfile, "w");

	if(fp == 0)
		return -1;

	for_all_symbols(i, sym){
		if(sym->name == 0 || sym->type == S_UNKNOWN || sym->type  == S_OTHER)
			continue;

		fprintf(fp, "%s/", path);

		s = sym->name;
		while((c = *s++)){
			c = tolower(c);
			fputc((c == '_') ? '/' : c, fp);
		}

		fprintf(fp, ".h: %s\n", conf_header);
	}

	fclose(fp);

	printf("configuration header dependency file written to %s\n", depfile);

	return 0;
}
