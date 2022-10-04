/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_FS_H
#define KERNEL_FS_H


#include <kernel/ksignal.h>
#include <kernel/task.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mutex.h>


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
	 * 			allocations if required for individual file systems. The resulting
	 * 			descriptor is finally added to this_p's list.
	 *
	 * \param	start	File system node to start search from
	 * \param	path	Path to the target file
	 * \param	mode	Mode to consider when opening the file. Individual behaviour
	 * 					depends on the concrete file system implementation.
	 * \param	this_p	process that the file descriptor is allocated to
	 *
	 * \return	Id of the file descriptor allocated or a value smaller than 0 if an
	 * 			error occurred. In the latter case errno is set appropriately.
	 *
	 * \post	If the target file has been found a file descriptor is allocated
	 * 			using fs_fd_alloc().
	 */
	int (*open)(struct fs_node_t *start, char const *path, f_mode_t mode, struct process_t *this_p);

	/**
	 * \brief	Release the given file descriptor and revert file system dependent
	 * 			allocations performed through the respective open() callback. Also
	 * 			remove the descriptor from this_p's list.
	 *
	 * \param	fd		pointer to target file descriptor
	 * \param	this_p	process that the file descriptor is allocated to
	 *
	 * \return	0 on success and a value smaller than 0 if an error occurred. In
	 * 			the latter case errno is set appropriately.
	 *
	 * \post	If no error occurred the target file descriptor is released using
	 * 			fs_fd_free().
	 */
	int (*close)(struct fs_filed_t *fd, struct process_t *this_p);

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
	 * \param	n			size of the memory pointed to by data
	 *
	 * \return	0 on success. If an error occurred a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*ioctl)(struct fs_filed_t *fd, int request, void *data, size_t n);

	/**
	 * \brief	Perform the requested command on the given file descriptor fd.
	 *
	 * \param	fd			target file descriptor
	 * \param	request		Command to perform. Behaviour depends on the individual
	 * 						file system implementation.
	 * \param	data		pointer to additional data that might be required to
	 * 						perform the request (kernel memory)
	 *
	 * \return	0 on success. If an error occurred a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*fcntl)(struct fs_filed_t *fd, int cmd, void *data);

	/**
	 * \brief	Get a user-space address for the memory associated with the given
	 * 			file descriptor.
	 *
	 * \param	fd		target file descriptor
	 * \param	n		size of the mapping
	 *
	 * \return	User-space address on success, 0x0 on error.
	 */
	void * (*mmap)(struct fs_filed_t *fd, size_t n);

	/**
	 * \brief	Remove the node specified through start and path from the file
	 * 			system.
	 *
	 * \param	start	root node to start the search for the target node
	 * \param	path	path to the target node
	 *
	 * \return	0 if the target node has been removed. On error a value smaller
	 * 			than 0 is returned and errno is set appropriately.
	 */
	int (*node_rm)(struct fs_node_t *start, char const *path);

	/**
	 * \brief	Return the node specified through start and path.
	 *
	 * \param	start	root node to start the search for the target node
	 * \param	path	path to the target node
	 *
	 * \return	pointer to the target node on success. On error 0x0 is returned and
	 * 			errno is set appropriately.
	 */
	struct fs_node_t *(*node_find)(struct fs_node_t *start, char const *path);
} fs_ops_t;

typedef struct fs_t{
	struct fs_t *prev,
				*next;

	int id;
	fs_ops_t ops;
} fs_t;

// file system node types
typedef struct fs_node_t{
	struct fs_node_t *prev,
					 *next;

	int fs_id;
	fs_ops_t *ops;

	void *data;
	unsigned int ref_cnt;
	file_type_t type;

	mutex_t mtx;
	ksignal_t datain_sig;
	uint32_t timeout_us;

	struct fs_node_t *childs,
					 *parent;

	char name[];
} fs_node_t;

// file descriptor types
typedef struct fs_filed_t{
	struct fs_filed_t *prev,
					  *next;

	int id;

	size_t fp;
	fs_node_t *node;
	f_mode_t mode;

	mutex_t mtx;
} fs_filed_t;


/* prototypes */
// file system operations
int fs_register(fs_ops_t *ops);
void fs_release(int id);

void fs_lock(void);
void fs_unlock(void);

// file descriptor operations
fs_filed_t *fs_fd_alloc(fs_node_t *node, struct process_t *this_p, f_mode_t mode);
int fs_fd_dup(fs_filed_t *fd, int id, struct process_t *this_p);
void fs_fd_free(fs_filed_t *fd, struct process_t *this_p);

fs_filed_t *fs_fd_acquire(int id, struct process_t *this_p);
void fs_fd_release(fs_filed_t *fd);

int fs_fd_wait(fs_filed_t *fd, ksignal_t *sig, mutex_t *mtx);

// file node operations
fs_node_t *fs_node_create(fs_node_t *parent, char const *name, size_t name_len, file_type_t type, void *data, int fs_id);
int fs_node_destroy(fs_node_t *node);
int fs_node_find(fs_node_t **start, char const **path);


#endif // KERNEL_FS_H
