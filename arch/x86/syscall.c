/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/syscall.h>


/* global functions */
int x86_sc_overlay_call(sc_num_t num, void *param, overlay_location_t loc, overlay_t *overlays){
	if(overlays[num].call == 0x0 || (overlays[num].loc & loc) == 0)
		return 0;

	return overlays[num].call(param);
}
