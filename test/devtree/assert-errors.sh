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
assert_dts=$(find ${SCRIPT_PATH} -name 'assert*.dts')

[ "${assert_dts}" == "" ] && { echo -e "\033[31merror\033[0m no assert devtree scripts found"; exit 1; }

for dts in ${assert_dts}
do
	ofile="recent/${dts}.c"
	out=$(${dtc} ${dts} --format=c --output=${ofile} 2>&1)  || { echo ${out}; exit 1; }
	out=$(cc -Irecent -Iinclude -c ${ofile} 2>&1)

	verify_errors "${dts}" "${out}" "${dts}\\|_Static_assert"
done
