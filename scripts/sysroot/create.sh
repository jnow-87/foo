#!/bin/bash
#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# argument assignemnt
build_dir=${1}
sysroot=${2}
arch_hdr=${3}
cfg_hdr=${4}
kernel_name=${5}
lib_name=${6}


# defines
verbose=0
sysroot_dir=${build_dir}/${sysroot}


# src include paths to copy to sysroot
src_dirs="include/sys include/lib include/arch"


# functions
function copy(){
	src=${1}
	tgt=${2}

	[ -e ${tgt} ] || mkdir -p ${tgt}
	[ $verbose -gt 0 ] && echo copy ${src}
	cp -r ${src} ${tgt}
}


# script
# create sysroot directory
echo create sysroot at \"${sysroot_dir}\"
[ -e ${sysroot_dir} ] && rm -r ${sysroot_dir}

# copy files
copy ${build_dir}/${kernel_name} ${sysroot_dir}
copy ${build_dir}/lib/${lib_name} ${sysroot_dir}/lib
copy ${cfg_hdr} ${sysroot_dir}/usr/include/config

for dir in ${src_dirs}
do
	copy ${dir} ${sysroot_dir}/usr/include
done

[ ${verbose} -gt 0 ] && echo -e "\n"

# replace BUILD_ARCH_HEADER macro by actual architecture header
sed -i -e "s:\bBUILD_ARCH_HEADER\b:<${arch_hdr}>:" ${sysroot_dir}/usr/include/arch/arch.h
