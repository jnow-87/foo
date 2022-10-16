/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <kernel/ktask.h>
#include <kernel/ktaskqueue.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/mutex.h>
#include <sys/list.h>


/* static variables */
static ktask_t *tasks = 0x0;
static mutex_t task_mtx = MUTEX_INITIALISER();


/* local/static prototypes */
/* global functions */
/**
 * \brief	create a kernel task
 *
 * \param	hdlr		actual task implementation
 * \param	payload		data required by the task
 * 						NOTE the payload is copied and the actual pointer passed
 * 							 to the task implementation must not be freed
 *
 * \param	size		size of the payload
 * \param	dep_queue	Optional dependency queue of tasks that defines dependencies
 * 						between	tasks, i.e. the task is not executed as long as not
 * 						all preceding tasks in that queue have been processed.
 * 						If 0x0 is passed the task is assumed to have no dependencies
 * 						to other tasks
 *
 * \param	recurring	recurring tasks spawn a new task immediately after the current
 * 						instance has been processed
 *
 * \return	0 on success, negative value on error, with errno being set appropriately
 * 				E_NOMEM		not enough memory to allocate the task
 * 				E_NOIMP		kernel task queues are disabled
 */
int ktask_create(ktask_hdlr_t hdlr, void *payload, size_t size, ktask_queue_t *dep_queue, bool recurring){
	ktask_t *task;


	/* create task */
	task = kmalloc(sizeof(ktask_t) + size);

	if(task == 0x0)
		goto err_0;

	task->hdlr = hdlr;
	task->flags = TASK_READY | (recurring ? TASK_RECURRING : 0);

	if(payload && size)
		memcpy(task->payload, payload, size);

#ifdef CONFIG_KERNEL_KTASK_QUEUE
	task->dep_queue = dep_queue;
#endif // CONFIG_KERNEL_KTASK_QUEUE

	/* enqueue task */
	mutex_lock(&task_mtx);

#ifdef CONFIG_KERNEL_KTASK_QUEUE
	if(dep_queue != 0x0 && ktask_queue_enqueue(dep_queue, task) != 0)
		goto err_1;
#else
	if(dep_queue != 0x0){
		WARN("task queues are disabled\n");

		goto_errno(err_1, E_NOIMP);
	}
#endif // CONFIG_KERNEL_KTASK_QUEUE

	list_add_tail(tasks, task);

	mutex_unlock(&task_mtx);

	return 0;


err_1:
	kfree(task);

err_0:
	return -errno;
}

/**
 * \brief	signal a task's completion and activate subsequent tasks if
 * 			the task is part of a dependency queue
 *
 * \param	task	the task to complete
 */
void ktask_complete(ktask_t *task){
	mutex_lock(&task_mtx);

#ifdef CONFIG_KERNEL_KTASK_QUEUE
	if(task->dep_queue)
		ktask_queue_dequeue(task->dep_queue);
#endif // CONFIG_KERNEL_KTASK_QUEUE

	if(!(task->flags & TASK_RECURRING))
		kfree(task);
	else
		list_add_tail(tasks, task);

	mutex_unlock(&task_mtx);
}

/**
 * \brief	prevent the given task from being executed it not already in execution
 *
 * \param	task	task to cancel
 */
void ktask_cancel(ktask_t *task){
	mutex_lock(&task_mtx);

	// only delete tasks that are still in the global task list, since all other
	// tasks are currently processed and will delete themselves
	// in the latter case ensure the task does not access an invalid queue
	if(list_contains(tasks, task)){
		list_rm(tasks, task);
		kfree(task);
	}
#ifdef CONFIG_KERNEL_KTASK_QUEUE
	else
		task->dep_queue = 0x0;
#endif // CONFIG_KERNEL_KTASK_QUEUE

	mutex_unlock(&task_mtx);
}

/**
 * \brief	get a task to be executed
 *
 * \return	0x0 if no further task is ready
 * 			tasks pointer otherwise
 */
ktask_t *ktask_next(void){
	ktask_t *task;


	mutex_lock(&task_mtx);

	task = list_first(tasks);

	/* search for ready task */
	while(task && !(task->flags & TASK_READY))
		task = task->next;

	/* remove task from global queues
	 *	NOTE tasks remain in local queues to ensure flush operations
	 *		 will wait for the outstanding task to finish
	 */
	if(task)
		list_rm(tasks, task);

	mutex_unlock(&task_mtx);

	return task;
}
