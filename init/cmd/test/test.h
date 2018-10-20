#ifndef CMD_TEST_H
#define CMD_TEST_H


/* macros */
#define test(_name, _exec, _descr) \
	static char const test_name_##_exec[]  __used = _name; \
	static char const test_descr_##_exec[]  __used = _descr; \
	static test_t test_##_exec __section(".tests") __used = { .name = test_name_##_exec, .descr = test_descr_##_exec, .exec = _exec }


/* types */
typedef struct{
	char const *name;
	char const *descr;
	int (*exec)(void);
} test_t;


#endif // CMD_TEST_H
