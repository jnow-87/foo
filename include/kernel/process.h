#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H


#include <config/config.h>
#include <kernel/thread.h>
#include <kernel/binloader.h>
#include <kernel/fs.h>
#include <sys/memblock.h>
#include <sys/process.h>
#include <sys/mutex.h>


/* incomplete types */
struct thread_t;
struct page_t;


/* types */
typedef struct{
#ifdef CONFIG_KERNEL_VIRT_MEM
	memblock_t *addr_space;		// memblock struct to manage processes virtual memory space
#endif // CONFIG_KERNEL_VIRT_MEM

	struct page_t *pages;		// list to manage allocated addresses in user memory (umalloc, ufree in memory.h)
} process_mem_t;

typedef struct process_t{
	pid_t pid;

	unsigned int affinity;
	int priority;

	char *name,
		 *args;

	process_mem_t memory;

	struct thread_t *threads;
	fs_filed_t *fds;
	fs_node_t *cwd;

	mutex_t mtx;

	struct process_t *prev,
					 *next;
} process_t;


/* prototypes */
process_t *process_create(void *binary, bin_type_t bin_type, char const *name, char const *args, fs_node_t *cwd);
void process_destroy(process_t *this_p);


#endif // KERNEL_PROCESS_H
