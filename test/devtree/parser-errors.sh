#!/bin/bash
#
# Copyright (C) 2025 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})


source ${SCRIPT_PATH}/helper.sh


dtc=recent/scripts/devtree/compiler/dtc
error_dts=$(find ${SCRIPT_PATH} -name 'error*.dts')

[ "${error_dts}" == "" ] && { echo -e "\033[31merror\033[0m no error devtree scripts found"; exit 1; }

for dts in ${error_dts}
do
	verify_errors "${dts}" "$(${dtc} ${dts} 2>&1)" "${dts}"
done
