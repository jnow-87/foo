#include <kernel/memory.h>
#include <kernel/task.h>
#include <kernel/sched.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/list1.h>


/* static variables */
static ktask_t *tasks = 0x0;
static ktask_queue_t *queues = 0x0;
static mutex_t task_mtx = MUTEX_INITIALISER();


/* global functions */
/**
 * \brief	create a kernel task
 *
 * \param	hdlr		actual task implementation
 * \param	data		data required by the task
 * 						NOTE the data are copied and the actual pointer passed
 * 							 to the task implementation must not be freed
 *
 * \param	size		size of the data
 * \param	queue		Optional dependency queue of tasks that defines dependencies
 * 						between	tasks, i.e. the task is not executed as long as not
 * 						all preceding tasks in that queue have been processed.
 * 						If 0x0 is passed the task is assumed to have no dependencies
 * 						to other tasks
 *
 * \param	recurring	recurring tasks spawn a new task immediately after the current
 * 						instance has been processed
 *
 * \return	0 on success
 * 			<0 on error, with errno being set appropriately
 * 				E_NOMEM		not enough memory to allocate the task
 */
int ktask_create(ktask_hdlr_t hdlr, void *data, size_t size, ktask_queue_t *queue, bool recurring){
	ktask_t *task;


	/* create task */
	task = kmalloc(sizeof(ktask_t) + size);

	if(task == 0x0)
		return_errno(E_NOMEM);

	task->hdlr = hdlr;
	task->queue = queue;
	task->queue_next = 0x0;
	task->flags = TASK_READY | (recurring ? TASK_RECURRING : 0);

	if(data && size)
		memcpy(task->data, data, size);

	/* enqueue task */
	mutex_lock(&task_mtx);

	// add to local task queue
	if(queue != 0x0){
		if(!list_empty(queue->head))
			task->flags &= ~TASK_READY;

		__list1_add_tail(queue->head, queue->tail, task, queue_next);
	}

	// add to global task queue
	list_add_tail(tasks, task);

	mutex_unlock(&task_mtx);

	return E_OK;
}

/**
 * \brief	signal a task's completion and activate subsequent tasks if
 * 			the task is part of an dependency queue
 *
 * \param	task	the task to complete
 */
void ktask_complete(ktask_t *task){
	ktask_queue_t *queue;
	bool queue_valid;


	mutex_lock(&task_mtx);

	queue = task->queue;
	queue_valid = (queue && list_contains(queues, queue));

	/* update local task queue */
	// check if the task's queue is still valid or has been destroyed
	if(queue_valid){
		// remove task from local queue
		__list1_rm_head(queue->head, queue->tail, queue_next);

		// activate next task in queue
		if(task->queue_next)
			task->queue_next->flags |= TASK_READY;
	}

	/* re-enqueue recurring tasks and delete non-recurring ones */
	if((task->flags & TASK_RECURRING) && (task->queue == 0x0 || queue_valid)){
		list_add_tail(tasks, task);
		__list1_add_tail(queue->head, queue->tail, task, queue_next);
	}
	else
		kfree(task);

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

/**
 * \brief	create a task dependency queue
 *
 * \return	pointer to the queue
 * 			0x0 in case of an error, with errno being set appropriately
 * 				E_NOMEM		not enough memory to allocate the queue
 */
ktask_queue_t *ktask_queue_create(void){
	ktask_queue_t *queue;


	queue = kmalloc(sizeof(ktask_queue_t));

	if(queue == 0x0)
		goto_errno(err, E_NOMEM);

	queue->head = 0x0;
	queue->tail = 0x0;

	mutex_lock(&task_mtx);
	list_add_tail(queues, queue);
	mutex_unlock(&task_mtx);

	return queue;

err:
	return 0x0;
}

/**
 * \brief	destroy the given task dependency queue
 *
 * \param	queue	queue to destroy
 */
void ktask_queue_destroy(ktask_queue_t *queue){
	ktask_t *task;


	mutex_lock(&task_mtx);

	/* destroy outstanding tasks */
	while(queue->head){
		task = list_first(queue->head);

		if(task)
			__list1_rm_head(queue->head, queue->tail, queue_next);

		// only delete tasks that are still in the global
		// task list, since all other tasks are currently
		// processed and will delete themselves
		if(list_contains(tasks, task)){
			list_rm(tasks, task);
			kfree(task);
		}
	}

	/* free the queue */
	list_rm(queues, queue);
	kfree(queue);

	mutex_unlock(&task_mtx);
}

/**
 * \brief	flush the given dependency queue, i.e. wait for it's tasks
 * 			to be processed
 *
 * \param	queue	queue to flush
 */
void ktask_queue_flush(ktask_queue_t *queue){
	while(queue->head)
		sched_yield();
}
