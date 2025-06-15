#!/bin/bash
#
# Copyright (C) 2025 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



function verify_errors(){
	dts="$1"
	err_log="$2"
	log_err_indicator="$3"


	cat ${dts} | grep "error-string" | sed -e 's:// error-string\: \(.*\):\1:' | while read -r error
	do
		if ! echo ${err_log} | grep "${error}" > /dev/null;then
			echo -e "\033[31merror\033[0m:\033[35m${dts}\033[0m: error-string \"${error}\" not reported when compiling devtree script"
			echo -e "  compiler output:\n$(echo "${err_log}" | sed -e 's:^:    :')"
			exit 1
		fi
	done

	[ $? -eq 0 ] || exit 1

	reported_err=$(echo "${err_log}" | grep "${log_err_indicator}" | wc -l)
	expected_err=$(cat ${dts} | grep "error-string" | wc -l)

	if [ ! ${reported_err} -eq ${expected_err} ];then
		echo -e "\033[31merror\033[0m:\033[35m${dts}\033[0m: mismatching number of expected ${expected_err} and reported ${reported_err} errors"
		exit 1
	fi
}
