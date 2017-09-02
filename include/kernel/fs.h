#ifndef KERNEL_FS_H
#define KERNEL_FS_H


#include <sys/file.h>
#include <sys/types.h>


/* incomplete types */
struct process_t;


/* types */
// file system node types
typedef struct fs_node_t{
	int fs_id;
	char *name;

	void *data;
	unsigned int ref_cnt;
	bool is_dir;

	struct fs_node_t *childs,
					 *parent;

	struct fs_node_t *prev,
					 *next;
} fs_node_t;

// file descriptor types
typedef struct fs_filed_t{
	int id;
	unsigned int fp;
	fs_node_t *node;

	struct fs_filed_t *prev,
					  *next;
} fs_filed_t;

// file system types
typedef struct{
																				/**  \return	success			error */
	int (*open)(fs_node_t *start, char const *path, f_mode_t mode);				/**< \return	descriptor id	<0		allocate file descriptor using fs_mkfd() for given path */
	int (*close)(fs_filed_t *fd);												/**< \return	E_OK			<0		free file descriptor using fs_rmfd() */
	size_t (*read)(fs_filed_t *fd, void *buf, size_t n);						/**< \return	#bytes read		0		copy data to buffer */
	size_t (*write)(fs_filed_t *fd, void *buf, size_t n);						/**< \return	#bytes written	0		copy data from buffer */
	int (*ioctl)(fs_filed_t *fd, int request, void *data);						/**< \return	E_OK			<0 		file dependent operation */
	int (*fcntl)(fs_filed_t *fd, int cmd, void *data);							/**< \return	E_OK 			<0 		file dependent operation */
	int (*rmnode)(fs_node_t *start, char const *path);							/**< \return	E_OK			<0		remove given node from file system */
	int (*chdir)(fs_node_t *start, char const *path);							/**< \return	E_OK			<0		change current working directory of current process */
} fs_ops_t;

typedef struct fs_t{
	int id;
	fs_ops_t *ops;

	struct fs_t *prev,
				*next;
} fs_t;


/* prototypes */
// file system operations
int fs_register(fs_ops_t *ops);
int fs_release(int fs_id);

fs_ops_t *fs_get_ops(int fs_id);

// file operations
fs_filed_t *fs_mkfd(fs_node_t *node);
void fs_rmfd(fs_filed_t *fd);


#endif // KERNEL_FS_H
