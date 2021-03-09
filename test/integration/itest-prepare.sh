#!/bin/bash
#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



readonly CONFIG_VAR="CONFIG_TEST_INT_FS_EXPORT_ROOT"

fs_export_root=$(cat .config | grep "${CONFIG_VAR}" | cut -d '=' -f 2 | sed -e 's:"::g')

[ "${fs_export_root}" != "" ] || { echo "undefined config variable \"${CONFIG_VAR}\""; exit 1; }


echo "removing brickos file system export at ${fs_export_root}"
rm -rf ${fs_export_root}
