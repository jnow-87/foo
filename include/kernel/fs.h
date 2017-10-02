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
	/**
	 * \brief	Create file descriptor for the file specified through start and path,
	 * 			starting the search at file system node start. Also perform any
	 * 			allocations if required for individual file systems.
	 *
	 * \param	start	File system node to start search from
	 * \param	path	Path to the target file
	 * \param	mode	Mode to consider when opening the file. Individual behaviour
	 * 					depends on the concrete file system implementation.
	 *
	 * \return	Id of the file descriptor allocated or a value smaller than 0 if an
	 * 			error occurred. In the latter case errno is set appropriately.
	 *
	 * \post	If the target file has been found a file descriptor is allocated
	 * 			using fs_fd_alloc().
	 */
	int (*open)(struct fs_node_t *start, char const *path, f_mode_t mode);

	/**
	 * \brief	Release the given file descriptor and revert file system dependent
	 * 			allocations performed through the respective open() callback.
	 *
	 * \param	fd	pointer to target file descriptor
	 *
	 * \return	E_OK on success and a value smaller than 0 if an error occurred. In
	 * 			the latter case errno is set appropriately.
	 *
	 * \post	If no error occurred the target file descriptor is released using
	 * 			fs_fd_free().
	 */
	int (*close)(struct fs_filed_t *fd);

	/**
	 * \brief	Copy utmost n bytes from the file associated with fd to buffer buf.
	 *
	 * \param	fd		target file descriptor
	 * \param	buf		buffer to copy to (kernel memory)
	 * \param	n		maximum number to copy to buf
	 *
	 * \return	Number of copied bytes. If an error occurred 0 is returned and error
	 * 			is set appropriately.
	 */
	size_t (*read)(struct fs_filed_t *fd, void *buf, size_t n);

	/**
	 * \brief	Copy utmost n bytes from buffer buf to the file associated with fd.
	 *
	 * \param	fd		target file descriptor
	 * \param	buf		buffer to copy from (kernel memory)
	 * \param	n		maximum number to copy to buf
	 *
	 * \return	Number of copied bytes. If an error occurred 0 is returned and error
	 * 			is set appropriately.
	 */
	size_t (*write)(struct fs_filed_t *fd, void *buf, size_t n);

	/**
	 * \brief	Perform the given device manipulation defined by request on the file
	 * 			associated with fd.
	 *
	 * \param	fd			target file descriptor
	 * \param	request		Operation to perform. Behaviour depends on the individual
	 * 						file system implementation.
	 * \param	data		pointer to additional data that might be required to
	 * 						perform the request (kernel memory)
	 *
	 * \return	E_OK on succes. If an error occured a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*ioctl)(struct fs_filed_t *fd, int request, void *data);

	/**
	 * \brief	Perform the requested command on the given file descriptor fd.
	 *
	 * \param	fd			target file descriptor
	 * \param	request		Command to perform. Behaviour depends on the individual
	 * 						file system implementation.
	 * \param	data		pointer to additional data that might be required to
	 * 						perform the request (kernel memory)
	 *
	 * \return	E_OK on succes. If an error occured a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*fcntl)(struct fs_filed_t *fd, int cmd, void *data);

	/**
	 * \brief	Remove the node specified through start and path from the file
	 * 			system.
	 *
	 * \param	start		root node to start the search for the target node
	 * \param	path		path to the target node
	 *
	 * \return	E_OK if the target node has been removed. On error a value smaller
	 * 			than 0 is returned and errno is set appropriately.
	 */
	int (*rmnode)(struct fs_node_t *start, char const *path);

	/**
	 * \brief	Change the working diretory of the parent process of the current
	 * 			thread to the node specified through start and path.
	 *
	 * \param	start		root node to start the search for the target node
	 * \param	path		path to the target node
	 *
	 * \return	E_OK if the target node has been removed. On error a value smaller
	 * 			than 0 is returned and errno is set appropriately.
	 */
	int (*chdir)(struct fs_node_t *start, char const *path);
} fs_ops_t;

typedef struct fs_t{
	int id;
	fs_ops_t ops;

	struct fs_t *prev,
				*next;
} fs_t;

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
// file system operations
int fs_register(fs_ops_t *ops);

// file operations
fs_filed_t *fs_fd_alloc(fs_node_t *node);
void fs_fd_free(fs_filed_t *fd);

// file node operations
fs_node_t *fs_node_alloc(fs_node_t *parent, char const *name, size_t name_len, bool is_dir, int fs_id);
int fs_node_free(fs_node_t *node);
int fs_node_find(fs_node_t **start, char const **path);



#endif // KERNEL_FS_H
