#include <kernel/memory.h>
#include <kernel/task.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/types.h>
#include <sys/list.h>


/* static variables */
static ktask_t *tasks = 0x0;
static mutex_t task_mtx = MUTEX_INITIALISER();


/* global functions */
int ktask_create(ktask_hdlr_t hdlr, void *data, size_t size){
	ktask_t *task;


	task = kmalloc(sizeof(ktask_t) + size);

	if(task == 0x0)
		return -errno;

	task->hdlr = hdlr;
	memcpy(task->data, data, size);

	list_add_tail_safe(tasks, task, &task_mtx);

	return E_OK;
}

void ktask_destroy(ktask_t *task){
	kfree(task);
}

ktask_t *ktask_next(void){
	ktask_t *task;


	mutex_lock(&task_mtx);

	task = list_first(tasks);

	if(task)
		list_rm(tasks, task);

	mutex_unlock(&task_mtx);

	return task;
}
