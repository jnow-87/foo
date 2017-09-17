#ifndef KERNEL_FS_H
#define KERNEL_FS_H


#include <sys/file.h>
#include <sys/types.h>


/* incomplete types */
struct process_t;
struct fs_filed_t;
struct fs_node_t;


/* types */
// file system types
typedef struct{
																					/**  \return	success			error */
	int (*open)(struct fs_node_t *start, char const *path, f_mode_t mode);				/**< \return	descriptor id	<0		allocate file descriptor using fs_fd_alloc() for given path */
	int (*close)(struct fs_filed_t *fd);												/**< \return	E_OK			<0		free file descriptor using fs_fd_free() */
	size_t (*read)(struct fs_filed_t *fd, void *buf, size_t n);							/**< \return	#bytes read		0		copy data to buffer */
	size_t (*write)(struct fs_filed_t *fd, void *buf, size_t n);						/**< \return	#bytes written	0		copy data from buffer */
	int (*ioctl)(struct fs_filed_t *fd, int request, void *data);						/**< \return	E_OK			<0 		file dependent operation */
	int (*fcntl)(struct fs_filed_t *fd, int cmd, void *data);							/**< \return	E_OK 			<0 		file dependent operation */
	int (*rmnode)(struct fs_node_t *start, char const *path);							/**< \return	E_OK			<0		remove given node from file system */
	int (*chdir)(struct fs_node_t *start, char const *path);							/**< \return	E_OK			<0		change current working directory of current process */
} fs_ops_t;

// file system node types
typedef struct fs_node_t{
	char *name;
	fs_ops_t *ops;

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
	size_t fp;
	fs_node_t *node;

	struct fs_filed_t *prev,
					  *next;
} fs_filed_t;


/* prototypes */
// file operations
fs_filed_t *fs_fd_alloc(fs_node_t *node);
void fs_fd_free(fs_filed_t *fd);

// file node operations
fs_node_t *fs_node_alloc(fs_node_t *parent, char const *name, size_t name_len, bool is_dir, fs_ops_t *ops);
int fs_node_free(fs_node_t *node);
int fs_node_find(fs_node_t **start, char const **path);



#endif // KERNEL_FS_H
