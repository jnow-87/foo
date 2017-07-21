#ifndef KERNEL_FS_H
#define KERNEL_FS_H


#include <sys/list.h>
#include <sys/fcntl.h>
#include <sys/errno.h>


/* types */
typedef enum{
	FS_DIR = 1,
	FS_FILE
} fs_node_type_t;

typedef struct fs_node_t{
	int fs_type;				// filesystem id	(0 reserved for root)
	fs_node_type_t node_type;

	unsigned int ref_cnt;		// reference counter
	char *name;					// name
	void *data;

	struct fs_node_t *childs,	// list of all childs
					 *parent;	// pointer to parent

	struct fs_node_t *prev,		// next, prev pointer to be used with list.h macros
					 *next;
} fs_node_t;

typedef struct fs_filed_t{
	unsigned int fd,
				 fp;

	void *data;
	fs_node_t *node;

	struct fs_filed_t *prev,
					  *next;
} fs_filed_t;

typedef struct{
	int (*open)(fs_node_t *start, char const *path, f_mode_t mode);				// \return	fd							find fs_node under start matching path (if a node is not found create it), return fd created with fs_mkfd()
	int (*close)(fs_filed_t *fd);												// \return	0 success, -1 error			free fd->data, then call fs_rmfd() (which cleans up the rest)
	int (*read)(fs_filed_t *fd, void *buf, unsigned int n);						// \return	number of bytes read		copy data into buf
	int (*write)(fs_filed_t *fd, void *buf, unsigned int n);					// \return	number of bytes written		copy data from buf
	int (*ioctl)(fs_filed_t *fd, int request, void *param);						// \return	0 success, -1 error
	int (*fcntl)(fs_filed_t *fd, int cmd, void *param);							// \return	0 success, -1 error
	int (*rmnode)(fs_node_t *start, char const *path);							// \return	0 success, -1 error			find node matching path and remove it from filesystem
	int (*chdir)(fs_node_t *start, char const *path);							// \return	0 success, -1 error			change processes CWD to node matching path
} fs_ops_t;

typedef struct fs_t{
	int fs_type;
	fs_ops_t *ops;				// pointer to the filesystems callback functions

	struct fs_t *prev,			// next, prev pointer to be used with list.h macros
				*next;
} fs_t;


/* prototyes */
int fs_register(fs_ops_t *ops);
int fs_unregister(int fs_type);
fs_ops_t *fs_get_ops(int fs_type);
fs_node_t *fs_get_cwd(fs_node_t *node);
fs_filed_t *fs_mkfd(fs_node_t *node, void *data);
void fs_rmfd(fs_filed_t *fd);
void fs_cleanup_fds(fs_filed_t *fds, unsigned int pid);


#endif // KERNEL_FS_H
