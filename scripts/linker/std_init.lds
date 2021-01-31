/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



__std_preinit_array_base = .;
*(.preinit_array)
__std_preinit_array_end = .;

__std_init_array_base = .;
*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*))
*(.init_array .ctors)
__std_init_array_end = .;

__std_fini_array_base = .;
*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*))
*(.fini_array .dtors)
__std_fini_array_end = .;
