#!/bin/bash
#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



VERBOSE=0


# \brief	remove trailing slashes from given string
function strip_slash(){
	echo $1 | sed 's:/*$::'
}

# \brief	copy (update) target with source
function copy(){
	src=$1
	tgt=$2

	[ -e ${tgt} ] || mkdir -p ${tgt}

	extra_flags=""
	[ $VERBOSE -gt 0 ] && extra_flags+="--verbose"

	cp ${extra_flags} -ur ${src} ${tgt} || { echo -e \"${src}\" \"${tgt}\"; }
}


sysroot=$(strip_slash $1)
arch_hdr=$2
kernel_name=$3
lib_name=$4

sysroot_kernel="${sysroot}"
sysroot_lib="${sysroot}/lib"
sysroot_inc="${sysroot}/usr/include"
sysroot_linker="${sysroot}"


echo "updating sysroot \"${sysroot}\""

# populate sysroot
copy "${kernel_name}"					"${sysroot_kernel}"
copy "${lib_name}"						"${sysroot_lib}"
copy "recent/config/"					"${sysroot_inc}"
copy "include/sys/"						"${sysroot_inc}"
copy "include/lib/"						"${sysroot_inc}"
copy "include/arch/"					"${sysroot_inc}"

lds="$(find recent/arch -name \*.lds | grep -v 'kernel')"
lds+=" $(find recent/scripts/linker -name \*.lds | grep -v 'kernel')"
lds+=" $(find scripts/linker -name \*.lds | grep -v '\(kernel_\|app_\)')"

copy "${lds}" "${sysroot_linker}/linker/"

# replace BUILD_ARCH_HEADER macro by actual architecture header
diff "include/arch/arch.h" "${sysroot_inc}/arch/arch.h" > /dev/null

if [ $? -eq 0 ];then
	sed -i -e "s:\bBUILD_ARCH_HEADER\b:<${arch_hdr}>:" "${sysroot_inc}/arch/arch.h"
fi
