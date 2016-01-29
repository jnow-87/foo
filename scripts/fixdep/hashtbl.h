#ifndef HASHTBL_H
#define HASHTBL_H


/* types */
struct item{
	struct item	*next;
	unsigned int	len;
	unsigned int	hash;
	char		name[0];
};


/* prototypes */
int hashtbl_lookup(const char *name, int len, unsigned int hash);
int hashtbl_add(const char *name, int len);
void hashtbl_clear(void);


#endif // HASHTBL_H
