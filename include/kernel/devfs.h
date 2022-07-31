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
	 * \return	E_OK on success. On error a value smaller than 0 is returned and
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
	 * \return	E_OK on success. On error a value smaller than 0 is returned and
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
	 * \param	data		additional data that might be required to perform the
	 * 						request (kernel memory)
	 * \param	n			size of the memory pointed to by data
	 *
	 * \return	E_OK on success. If an error occurred a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*ioctl)(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data, size_t n);

	/**
	 * \brief	Perform the requested command on the given file descriptor fd.
	 *
	 * \param	dev			target device
	 * \param	fd			target file descriptor
	 * \param	request		Command to perform. Behaviour depends on the individual
	 * 						file system implementation.
	 * \param	data		additional data that might be required to perform the
	 * 						request (kernel memory)
	 *
	 * \return	E_OK on success. If an error occurred a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*fcntl)(struct devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *data);
} devfs_ops_t;

typedef struct devfs_dev_t{
	devfs_ops_t ops;
	fs_node_t *node;

	void *data;
} devfs_dev_t;


/* prototypes */
devfs_dev_t *devfs_dev_register(char const *name, devfs_ops_t *ops, void *data);
int devfs_dev_release(devfs_dev_t *dev);


#endif // KERNEL_DEVFS_H
