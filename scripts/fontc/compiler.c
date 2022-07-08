/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "user.h"
#include "opts.h"
#include "parser.h"


/* local/static prototypes */
static int font_header(FILE *fp, font_header_t *hdr);
static int font_footer(FILE *fp);
static int font_letters(FILE *ifp, FILE *ofp, font_header_t *hdr);

static void convert_hor(char const *letter, uint8_t *hex, font_header_t *hdr);
static void convert_vert(char const *letter, uint8_t *hex, font_header_t *hdr);


/* global functions */
int compile_font(FILE *ifp, FILE *ofp, font_header_t *hdr){
	int r;


	r = font_header(ofp, hdr);
	r |= font_letters(ifp, ofp, hdr);
	r |= font_footer(ofp);

	if(r != 0)
		ERROR("writing file\n");

	return r;
}


/* local functions */
static int font_header(FILE *fp, font_header_t *hdr){
	return (
		fprintf(fp,
			"#include <sys/types.h>\n"
			"#include <sys/compiler.h>\n"
			"#include <sys/font.h>\n"
			"\n"
			"\n"
			"static font_t font_%s%s __linker_array(\"fonts\") = {\n"
			"\t.name = \"%s\",\n"
			"\n"
			"\t.first_char = %zu,\n"
			"\t.last_char = %zu,\n"
			"\t.height = %zu,\n"
			"\t.width = %zu,\n"
			"\n"
			"\t.data = {\n"
			,
			hdr->name, opts.vertical ? "_vert" : "",
			hdr->name,
			hdr->first_char,
			hdr->last_char,
			opts.vertical ? hdr->width : hdr->height,
			opts.vertical ? hdr->height : hdr->width
		) <= 0 ? -1 : 0
	);
}

static int font_footer(FILE *fp){
	return (
		fprintf(fp,
			"\t}\n"
			"};\n"
		) <= 0 ? -1 : 0
	);
}

static int font_letters(FILE *ifp, FILE *ofp, font_header_t *hdr){
	size_t i,
		   nsyms,
		   nhex = opts.vertical ? hdr->height : hdr->width;
	char letter[hdr->height * hdr->width];
	uint8_t hex[nhex];


	nsyms = 0;

	while(1){
		/* read */
		if(parse_letter(ifp, letter, hdr) != 0)
			return -1;

		if(feof(ifp))
			break;

		/* convert */
		if(opts.vertical)	convert_vert(letter, hex, hdr);
		else				convert_hor(letter, hex, hdr);

		/* write */
		if(fprintf(ofp, "\t\t(uint8_t []){ ") <= 0)
			return ERROR("writing file\n");

		for(i=0; i<nhex; i++){
			if(fprintf(ofp, "0x%2.2hhx%s", hex[i], ((i + 1 == nhex) ? " },\n" : ", ")) <= 0)
				return ERROR("writing file");
		}

		nsyms++;
	}

	if(nsyms != hdr->last_char - hdr->first_char + 1){
		return ERROR(
			"missmatch in number of symbols: header %zu - %zu + 1 = %zu, data %zu\n",
			hdr->last_char,
			hdr->first_char,
			hdr->last_char - hdr->first_char + 1,
			nsyms
		);
	}

	return 0;
}

static void convert_hor(char const *letter, uint8_t *hex, font_header_t *hdr){
	size_t w,
		   h;


	memset(hex, 0x0, hdr->width);

	for(w=0; w<hdr->width; w++){
		for(h=0; h<hdr->height; h++)
			hex[w] |= ((letter[hdr->width * h + w] == ' ') ? 0 : 1) << h;
	}
}

static void convert_vert(char const *letter, uint8_t *hex, font_header_t *hdr){
	size_t w,
		   h;


	memset(hex, 0x0, hdr->height);

	for(h=0; h<hdr->height; h++){
		for(w=0; w<hdr->width; w++)
			hex[h] |= ((letter[hdr->width * h + w] == ' ') ? 0 : 1) << (hdr->width - w - 1);
	}
}
