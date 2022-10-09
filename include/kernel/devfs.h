/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_DEVFS_H
#define KERNEL_DEVFS_H


#include <kernel/fs.h>
#include <sys/fcntl.h>


/* incomplete types */
struct devfs_dev_t;


/* types */
typedef struct{
	/**
	 * \brief	Perform allocations required if the device pointed to by dev is
	 * 			opened.
	 *
	 * \param	dev		target device
	 * \param	fd		readily allocated file descriptor that might be augmented with
	 * 					further data
	 * \param	mode	mode to consider when opening the device
	 *
	 * \return	0 on success. On error a value smaller than 0 is returned and
	 * 			errno is set appropriately.
	 */
	int (*open)(struct devfs_dev_t *dev, fs_filed_t *fd, f_mode_t mode);

	/**
	 * \brief	Revoke allocations performed by the respective open() call.
	 *
	 * \param	dev		target device
	 * \param	fd		Target file descriptor. The descriptor must not be released by
	 * 					this callback.
	 *
	 * \return	0 on success. On error a value smaller than 0 is returned and
	 * 			errno is set appropriately.
	 */
	int (*close)(struct devfs_dev_t *dev, fs_filed_t *fd);

	/**
	 * \brief	Read utmost n bytes from the target device and copy them to buf.
	 *
	 * \param	dev		target device
	 * \param	fd		target file descriptor
	 * \param	buf		buffer to copy to (kernel space)
	 * \param	n		maximum number of bytes to copy
	 *
	 * \return	Number of bytes read. On error 0 is returned and errno is set
	 * 			appropriately.
	 */
	size_t (*read)(struct devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);

	/**
	 * \brief	Write utmost n bytes to the target device.
	 *
	 * \param	dev		the target device
	 * \param	fd		target file descriptor
	 * \param	buf		buffer to copy from (kernel space)
	 * \param	n		maximum number of bytes to copy
	 *
	 * \return	Number of bytes written. On error 0 is returned and errno is set
	 * 			appropriately.
	 */
	size_t (*write)(struct devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);

	/**
	 * \brief	Perform the given device manipulation defined by request.
	 *
	 * \param	dev			the target device
	 * \param	fd			target file descriptor
	 * \param	request		Operation to perform. Behaviour depends on the individual
	 * 						file system implementation.
	 * \param	arg			additional data that might be required to perform the
	 * 						request (kernel memory)
	 * \param	n			size of the memory pointed to by arg
	 *
	 * \return	0 on success. If an error occurred a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*ioctl)(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);

	/**
	 * \brief	Perform the requested command on the given file descriptor fd.
	 *
	 * \param	dev			target device
	 * \param	fd			target file descriptor
	 * \param	request		Command to perform. Behaviour depends on the individual
	 * 						file system implementation.
	 * \param	arg			additional data that might be required to perform the
	 * 						request (kernel memory)
	 *
	 * \return	0 on success. If an error occurred a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*fcntl)(struct devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *arg);

	/**
	 * \brief	Get a user-space address for the memory associated with the target device.
	 *
	 * \param	dev		target device
	 * \param	fd		target file descriptor
	 * \param	n		size of the mapping
	 *
	 * \return	User-space address on success, 0x0 on error.
	 */
	void * (*mmap)(struct devfs_dev_t *dev, fs_filed_t *fd, size_t n);
} devfs_ops_t;

typedef struct devfs_dev_t{
	devfs_ops_t ops;
	fs_node_t *node;

	void *payload;
} devfs_dev_t;


/* prototypes */
devfs_dev_t *devfs_dev_register(char const *name, devfs_ops_t *ops, void *payload);
int devfs_dev_release(devfs_dev_t *dev);


#endif // KERNEL_DEVFS_H
