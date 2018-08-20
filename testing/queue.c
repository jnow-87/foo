#include <sys/queue.h>
#include <sys/types.h>
#include <sys/string.h>
#include <testing/testcase.h>


/* macros */
#define INIT_EL() \
	el0.next = (void*)0x1; \
	el1.next = (void*)0x1; \
	el2.next = (void*)0x1; \
	el3.next = (void*)0x1; \


/* types */
typedef struct queue_t{
	struct queue_t *next;

	int el;
	char s[5];
} queue_t;


/* static variables */
static queue_t el0 = { .el = 0, .s = "0" },
			   el1 = { .el = 1, .s = "1" },
			   el2 = { .el = 2, .s = "2" },
			   el3 = { .el = 3, .s = "3xx" };


/* local functions */
static int tc_queue_print(int log){
	tlog(log, "list element addresses\n");
	tlog(log, "el0 addr: %#x\n", &el0);
	tlog(log, "el1 addr: %#x\n", &el1);
	tlog(log, "el2 addr: %#x\n", &el2);
	tlog(log, "el3 addr: %#x\n", &el3);

	return 0;
}

test_case(tc_queue_print, "queue_print");


static int tc_queue_init(int log){
	unsigned int n;
	queue_t *head,
			*tail;


	n = 0;
	head = &el0;

	queue_init(head, tail);

	n += check_ptr(log, head->next, 0);
	n += check_ptr(log, tail, head);

	return -n;
}

test_case(tc_queue_init, "queue_init");


static int tc_queue_enqueue(int log){
	unsigned int n;
	queue_t *head,
			*tail;


	n = 0;
	head = 0;
	tail = 0;
	INIT_EL();

	// enqueue to empty queue
	queue_enqueue(head, tail, &el0);

	n += check_ptr(log, head, &el0);
	n += check_ptr(log, tail, &el0);
	n += check_ptr(log, el0.next, 0);

	// enqueue in non-empty queue
	queue_enqueue(head, tail, &el1);

	n += check_ptr(log, head, &el0);
	n += check_ptr(log, tail, &el1);
	n += check_ptr(log, el0.next, &el1);
	n += check_ptr(log, el1.next, 0);

	return -n;
}

test_case(tc_queue_enqueue, "queue_enqueue");


static int tc_queue_dequeue(int log){
	unsigned int n;
	queue_t *head,
			*tail,
			*el;


	n = 0;
	head = 0;
	tail = 0;
	INIT_EL();

	// prepare queue
	queue_enqueue(head, tail, &el0);
	queue_enqueue(head, tail, &el1);

	// dequeue from non-empty queue
	el = queue_dequeue(head, tail);

	n += check_ptr(log, head, &el1);
	n += check_ptr(log, tail, &el1);
	n += check_ptr(log, el, &el0);

	// dequeue last element
	el = queue_dequeue(head, tail);

	n += check_ptr(log, head, 0);
	n += check_ptr(log, tail, 0);
	n += check_ptr(log, el, &el1);

	// dequeue from empty lis from empty queue
	el = queue_dequeue(head, tail);

	n += check_ptr(log, head, 0);
	n += check_ptr(log, tail, 0);
	n += check_ptr(log, el, 0);

	return -n;
}

test_case(tc_queue_dequeue, "queue_dequeue");
