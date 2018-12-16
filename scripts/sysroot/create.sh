#!/bin/bash
#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#


#
# argument assignemnt
#
build_dir=${1}
sysroot=${2}
arch_hdr=${3}
kernel_name=${4}
lib_name=${5}


#
# defines
#
verbose=0
sysroot_dir=${build_dir}/${sysroot}


#
# src include paths to copy to sysroot
#
inc_dirs="recent/config include/sys include/lib include/arch"


#
# functions
#
function copy(){
	src=${1}
	tgt=${2}

	[ -e ${tgt} ] || mkdir -p ${tgt}
	[ $verbose -gt 0 ] && echo copy ${src}
	cp -r ${src} ${tgt}
}

#
# script
#

# create sysroot directory
echo create sysroot at \"${sysroot_dir}\"
[ -e ${sysroot_dir} ] && rm -r ${sysroot_dir}

# copy kernel and libsys
copy ${build_dir}/${kernel_name} ${sysroot_dir}
copy ${build_dir}/lib/${lib_name} ${sysroot_dir}/lib

# copy include directories
for dir in ${inc_dirs}
do
	copy ${dir} ${sysroot_dir}/usr/include
done

# copy linker scripts
copy scripts/linker ${sysroot_dir}
copy "$(find recent/arch -name \*.lds)" ${sysroot_dir}/linker


[ ${verbose} -gt 0 ] && echo -e "\n"

# replace BUILD_ARCH_HEADER macro by actual architecture header
sed -i -e "s:\bBUILD_ARCH_HEADER\b:<${arch_hdr}>:" ${sysroot_dir}/usr/include/arch/arch.h
