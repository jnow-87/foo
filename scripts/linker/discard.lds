/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* sections to not include to the binary */
/DISCARD/ : {
	*(.init4)	/* libgcc .data initialisation not needed */
	*(.discard)	/* brickos discard section */
}
