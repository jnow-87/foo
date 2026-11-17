#!/bin/bash
#
# Copyright (C) 2025 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})


function verify_errors(){
	local dts="$1"
	local err_log="$2"
	local log_err_indicator="$3"
	local fail=0


	cat ${dts} | grep "error-string" | sed -e 's:// error-string\: \(.*\):\1:' | while read -r error
	do
		if ! echo ${err_log} | grep "${error}" > /dev/null;then
			echo -e "\033[31merror\033[0m:\033[35m${dts}\033[0m: unreported error-string: \"${error}\""
			exit 1
		fi
	done

	let fail=$?
	expected_err=$(cat ${dts} | grep "error-string" | wc -l)
	reported_err=$(echo "${err_log}" | grep "${log_err_indicator}" | wc -l)

	if [ ! ${reported_err} -eq ${expected_err} ];then
		echo -e "\033[31merror\033[0m:\033[35m${dts}\033[0m: mismatching number of errors, expected ${expected_err}, reported ${reported_err}"
		let fail=1
	fi

	if [ ! ${fail} -eq 0 ];then
		echo -e "  \033[33mexpected errors\033[0m:"
		cat ${dts} | grep "error-string" | sed -e 's:.*error-string\: :    :'

		echo -e "\n  \033[33mdevtree compiler output\033[0m:"
		echo "${err_log}" | sed -e 's:^:    :'
		exit 1
	fi
}


dtc=recent/scripts/devtree/compiler/dtc
error_dts=$(find ${SCRIPT_PATH} -name 'error*.dts')

[ "${error_dts}" == "" ] && { echo -e "\033[31merror\033[0m no error devtree scripts found"; exit 1; }

for dts in ${error_dts}
do
	verify_errors "${dts}" "$(${dtc} ${dts} 2>&1)" "${dts}"
done
