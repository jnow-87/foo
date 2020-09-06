/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devtree.h>
#include <testing/testcase.h>


/* static variables */
// fake memory device tree
devtree_memory_t const memroot,
					   mem0,
					   mem0_child,
					   mem1;

devtree_memory_t const mem0 = {
	.name = "mem0",
	.base = (void*)0,
	.size = 0,
	.parent = &memroot,
	.childs = ((devtree_memory_t const *[]){ &mem0_child, 0x0 }),
};

devtree_memory_t const mem0_child = {
	.name = "mem0-child",
	.base = (void*)0,
	.size = 0,
	.parent = &mem0,
	.childs = ((devtree_memory_t const *[]){ 0x0 }),
};

devtree_memory_t const mem1 = {
	.name = "mem1",
	.base = (void*)0,
	.size = 0,
	.parent = &memroot,
	.childs = 0x0,
};

devtree_memory_t const memroot = {
	.name = "memory-root",
	.base = (void*)0,
	.size = 8405248,
	.parent = 0x0,
	.childs = ((devtree_memory_t const *[]){ &mem0, &mem1, 0x0 }),
};

// fake devices device tree
devtree_device_t const devroot,
					   dev0,
					   dev0_child,
					   dev1;

devtree_device_t const dev0 = {
	.name = "dev0",
	.compatible = "driver-dev0",
	.data = (void*)0,
	.parent = &devroot,
	.childs = ((devtree_device_t const *[]){ &dev0_child, 0x0 }),
};

devtree_device_t const dev0_child = {
	.name = "dev0-child",
	.compatible = "driver-dev0-child",
	.data = (void*)0,
	.parent = &dev0,
	.childs = ((devtree_device_t const *[]){ 0x0 }),
};

devtree_device_t const dev1 = {
	.name = "dev1",
	.compatible = "driver-dev1",
	.data = (void*)0,
	.parent = &devroot,
	.childs = 0x0,
};

devtree_device_t const devroot = {
	.name = "devory-root",
	.compatible = "",
	.data = (void*)0,
	.parent = 0x0,
	.childs = ((devtree_device_t const *[]){ &dev0, &dev1, 0x0 }),
};


/* local functions */
static int tc_devtree(int log){
	int n;


	n = 0;

	/* devtree_find_memory_by_name() */
	n += check_ptr(log, devtree_find_memory_by_name(&memroot, "memory-root"), &memroot);
	n += check_ptr(log, devtree_find_memory_by_name(&memroot, "mem0"), &mem0);
	n += check_ptr(log, devtree_find_memory_by_name(&memroot, "mem1"), &mem1);
	n += check_ptr(log, devtree_find_memory_by_name(&memroot, "mem0-child"), &mem0_child);

	n += check_ptr(log, devtree_find_memory_by_name(&memroot, "invalid"), 0x0);
	n += check_ptr(log, devtree_find_memory_by_name(&mem1, "zero-child"), 0x0);

	/* devtree_find_device_by_name() */
	n += check_ptr(log, devtree_find_device_by_name(&devroot, "devory-root"), &devroot);
	n += check_ptr(log, devtree_find_device_by_name(&devroot, "dev0"), &dev0);
	n += check_ptr(log, devtree_find_device_by_name(&devroot, "dev1"), &dev1);
	n += check_ptr(log, devtree_find_device_by_name(&devroot, "dev0-child"), &dev0_child);

	n += check_ptr(log, devtree_find_device_by_name(&devroot, "invalid"), 0x0);
	n += check_ptr(log, devtree_find_device_by_name(&dev1, "zero-child"), 0x0);

	/* devtree_find_device_by_comp() */
	n += check_ptr(log, devtree_find_device_by_comp(&devroot, ""), &devroot);
	n += check_ptr(log, devtree_find_device_by_comp(&devroot, "driver-dev0"), &dev0);
	n += check_ptr(log, devtree_find_device_by_comp(&devroot, "driver-dev1"), &dev1);
	n += check_ptr(log, devtree_find_device_by_comp(&devroot, "driver-dev0-child"), &dev0_child);

	n += check_ptr(log, devtree_find_device_by_comp(&devroot, "invalid"), 0x0);
	n += check_ptr(log, devtree_find_device_by_comp(&dev1, "zero-child"), 0x0);

	return -n;
}

test_case(tc_devtree, "devtree");
