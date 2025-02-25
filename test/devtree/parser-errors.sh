#!/bin/bash
#
# Copyright (C) 2025 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})


dtc=recent/scripts/devtree/compiler/dtc
error_dts=$(find ${SCRIPT_PATH} -name 'error*.dts')

[ "${error_dts}" == "" ] && { echo -e "\033[31merror\033[0m no error devtree scripts found"; exit 1; }

for dts in ${error_dts}
do
	out=$(${dtc} ${dts} 2>&1)

	cat ${dts} | grep "error-string" | sed -e 's:// error-string\: \(.*\):\1:' | while read -r error
	do
		if ! echo ${out} | grep "${error}" > /dev/null;then
			echo -e "\033[31merror\033[0m:\033[35m${dts}\033[0m: error-string \"${error}\" not reported when compiling devtree script"
			echo -e "  compiler output:\n$(echo "$out" | sed -e 's:^:    :')"
			exit 1
		fi
	done
done
