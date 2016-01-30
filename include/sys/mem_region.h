#ifndef SYS_MEM_REGION_H
#define SYS_MEM_REGION_H


#include <sys/types.h>


/* types */
typedef struct{
	bool valid;
	void* start;
	unsigned int size;
} mem_region_t;

typedef struct{
	mem_region_t *free_mem,		// pointer to array of mem_region_t holding portions of free memory
				 *alloc_mem;	// pointer to array of mem_region_t holding portions of allocated memory

	unsigned int nfree,			// number of valid entries in free_mem
				 nalloc,		// number of valid entries in alloc_mem
				 max_entries;	// maximum number of entries in free_mem and alloc_mem
} mem_info_t;


/* prototypes */
void* mem_region_alloc(uint32_t size, void* min_addr, mem_info_t* info);
void mem_region_merge(mem_region_t* toMerge, mem_info_t* info);
void mem_region_free(void* addr, mem_info_t* info);


#endif // SYS_MEM_REGION_H
