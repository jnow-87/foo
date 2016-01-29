#!/bin/bash

# argument assignemnt
build_dir=${1}
sysroot=${2}
arch=${3}

# defines
verbose=0
sysroot_dir=${build_dir}/${sysroot}
macro_replacer=${build_dir}/scripts/sysroot/macro_replacer

# src include paths to copy to sysroot
src_dirs="include/sys include/lib include/arch"

# which ARCH_* headers to consider
header="header mem timebase interrupt coreconfig spr syscall"

# which header files to generate
#	names have to match a name specified in ${header}
gen_header="timebase coreconfig spr"


function copy(){
	src=${1}
	tgt=${2}

	[ -e ${tgt} ] || mkdir -p ${tgt}
	[ $verbose -gt 0 ] && echo copy ${src}
	cp -r ${src} ${tgt}
}


# get macro information for ARCH_*
for h in ${header}
do
	# get upper-case name of ${h}
	H=$(echo ${h} | tr a-z A-Z)

	# grep header file used for ARCH_${H}
	val=$(${macro_replacer} print_macro | grep ARCH_${H} | cut -d ' ' -f 2)

	# check if header for ARCH_${H} is defined
	if [ "${val}" == "" ];then
		echo "macro ARCH_${H} undefined"
		exit 1
	fi

	# assign arch_${h} to header
	eval "arch_${h}=\"${val}\""
done

# create sysroot directory
echo create sysroot at \"${sysroot_dir}\"
[ -e ${sysroot_dir} ] && rm -r ${sysroot_dir}

# copy files
copy ${build_dir}/kimg.elf ${sysroot_dir}
copy ${build_dir}/lib/libsys.a ${sysroot_dir}/lib

for dir in ${src_dirs}
do
	# copy header files throug ${macro_replacer} to directly replace occurences
	# of #include ARCH_*
	for file in $(find ${dir} -name "*.h")
	do
		tgt_dir=${sysroot_dir}/usr/$(dirname ${file})
		[ -e ${tgt_dir} ] || mkdir -p ${tgt_dir}

		[ ${verbose} -gt 0 ] && echo -e "\t"${file}

		${macro_replacer} ${file} > ${sysroot_dir}/usr/${file}
	done
done

[ ${verbose} -gt 0 ] && echo -e "\n"

# create additional arch headers
echo generate additional header files
for h in ${gen_header}
do
	header_name=${sysroot_dir}/usr/include/arch/${h}.h

	if [ -e ${header_name} ];then
		echo -e "header \"${header_name}\" already exists"
		exit 1
	fi

	H=$(echo ${h} | tr a-z A-Z)
	eval "val=\$arch_${h}"
	[ $verbose -gt 0 ] && echo -e "\t"${header_name}

	echo -e \
"#ifndef GEN_SYS_${H}_H\n"\
"#define GEN_SYS_${H}_H\n\n\n"\
\
"#include ${val}\n\n\n"\
\
"#endif // GEN_SYS_${H}_H"\
	> ${header_name}
done
