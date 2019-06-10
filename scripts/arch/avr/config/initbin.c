/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include "avrconfig.h"


int config_initbin(void){
	// TODO fix to handle binary addresses that would exceed the 16-bit range
	CONFIG_PRINT(INIT_BINARY, CONFIG_INIT_BINARY / 2, "%u");

	return 0;
}
