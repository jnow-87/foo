/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devtree.h>
#include <test/test.h>


/* macros */
#define ROOT_PAYLOAD		(void*)0x1
#define DEV0_PAYLOAD		(void*)0x2
#define DEV0_CHILD_PAYLOAD	(void*)0x3
#define DEV1_PAYLOAD		(void*)0x4


/* static variables */
// fake device tree
devtree_node_t const dev0_child = {
	.name = "dev0-child",
	.compatible = "driver-dev0-child",
	.payload = DEV0_CHILD_PAYLOAD,
	.childs = ((devtree_node_t const *[]){ 0x0 }),
};

devtree_node_t const dev0 = {
	.name = "dev0",
	.compatible = "driver-dev0",
	.payload = DEV0_PAYLOAD,
	.childs = ((devtree_node_t const *[]){ &dev0_child, 0x0 }),
};

devtree_node_t const dev1 = {
	.name = "dev1",
	.compatible = "driver-dev1",
	.payload = DEV1_PAYLOAD,
	.childs = 0x0,
};

devtree_node_t const __dt_root = {
	.name = "root",
	.compatible = "",
	.payload = ROOT_PAYLOAD,
	.childs = ((devtree_node_t const *[]){ &dev0, &dev1, 0x0 }),
};

/* local functions */
TEST(devtree_find){
	int r = 0;


	/* by name */
	r |= TEST_PTR_EQ(devtree_find(0x0, "root"), ROOT_PAYLOAD);
	r |= TEST_PTR_EQ(devtree_find(0x0, "dev0"), DEV0_PAYLOAD);
	r |= TEST_PTR_EQ(devtree_find(0x0, "dev1"), DEV1_PAYLOAD);
	r |= TEST_PTR_EQ(devtree_find(0x0, "dev0-child"), DEV0_CHILD_PAYLOAD);

	r |= TEST_PTR_EQ(devtree_find(0x0, "invalid"), 0x0);

	/* by compatible */
	r |= TEST_PTR_EQ(devtree_find("", 0x0), ROOT_PAYLOAD);
	r |= TEST_PTR_EQ(devtree_find("driver-dev0", 0x0), DEV0_PAYLOAD);
	r |= TEST_PTR_EQ(devtree_find("driver-dev1", 0x0), DEV1_PAYLOAD);
	r |= TEST_PTR_EQ(devtree_find("driver-dev0-child", 0x0), DEV0_CHILD_PAYLOAD);

	r |= TEST_PTR_EQ(devtree_find("invalid", 0x0), 0x0);

	/* compatible and name combined */
	r |= TEST_PTR_EQ(devtree_find(0x0, 0x0), ROOT_PAYLOAD);
	r |= TEST_PTR_EQ(devtree_find("driver-dev0", "dev0"), DEV0_PAYLOAD);
	r |= TEST_PTR_EQ(devtree_find("driver-dev0-wrong", "dev0"), 0x0);
	r |= TEST_PTR_EQ(devtree_find("driver-dev0", "dev0-wrong"), 0x0);

	return -r;
}
