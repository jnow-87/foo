#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H


#include <kernel/page.h>
#include <kernel/thread.h>
#include <kernel/fs.h>
#include <sys/mem_region.h>


/* macros */
#define MAX_MEMORY_ENTRIES	32


/* incomplete types */
struct thread_t;


/* types */
typedef struct{
	mem_info_t addr_space;		// mem_info struct to manage processes virtual memory space
	page_t *pages;				// list to manage allocated addresses in user memory (umalloc, ufree in umem.h)
} process_mem_t;

typedef struct process_t{
	unsigned int pid,
				 lpid,
				 affinity,
				 priority;

	unsigned int argc;
	char **argv;

	char *name;

	process_mem_t memory;

	struct thread_t *threads;
	fs_filed_t *fds;
	fs_node_t *cwd;

	page_flags_t page_flags;

	struct process_t *next,
					 *prev;
} process_t;


/* prototypes */
process_t *process_create(void *elf_location, char const *name, unsigned int priority, unsigned int affinity, unsigned int argc, char **argv, fs_node_t *cwd);
void process_destroy(process_t *this_p);


#endif // KERNEL_PROCESS_H
