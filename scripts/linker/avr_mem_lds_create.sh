#!/bin/bash
#
# \brief	script to generate linker scripts for the avr architecture
#			containing the target controller memory configuration
#
# \param	1	config file derived from Kconfig
# \param	2	output linker file name


####
## arguments
####

if [ $# -lt 2 ];then
	printf "usage: %s <config> <output>\n\n" $(basename $0)
	printf "%15s    %s\n" "<config>" "config file derived from Kconfig"
	printf "%15s    %s\n" "<output>" "output linker file name"
	exit 1
fi

config=$1
ofile=$2

if [ ! -e "${config}" ];then
	printf "\tconfig file \"%s\" does not exist\n" ${config}
	exit 1
fi

####
## configuration
####

## get CONFIG-variables
eval $(cat ${config} | grep CONFIG_KERNEL_TEXT_BASE)
eval $(cat ${config} | grep CONFIG_KERNEL_TEXT_SIZE)
eval $(cat ${config} | grep CONFIG_KERNEL_DATA_BASE)
eval $(cat ${config} | grep CONFIG_KERNEL_DATA_SIZE)
eval $(cat ${config} | grep CONFIG_APP_TEXT_BASE)
eval $(cat ${config} | grep CONFIG_APP_TEXT_SIZE)
eval $(cat ${config} | grep CONFIG_APP_DATA_BASE)
eval $(cat ${config} | grep CONFIG_APP_DATA_SIZE)

## init section attributes
kernel_flash_name="flash_kernel"
kernel_flash_base=$(printf "%u" ${CONFIG_KERNEL_TEXT_BASE})
kernel_flash_size=$(printf "%u" ${CONFIG_KERNEL_TEXT_SIZE})
kernel_data_name="data_kernel"
kernel_data_base=$(printf "%u" ${CONFIG_KERNEL_DATA_BASE})
kernel_data_size=$(printf "%u" ${CONFIG_KERNEL_DATA_SIZE})

app_flash_name="flash_app"
app_flash_base=$(printf "%u" ${CONFIG_APP_TEXT_BASE})
app_flash_size=$(printf "%u" ${CONFIG_APP_TEXT_SIZE})
app_data_name="data_app"
app_data_base=$(printf "%u" ${CONFIG_APP_DATA_BASE})
app_data_size=$(printf "%u" ${CONFIG_APP_DATA_SIZE})

## init linker file template
template_header="MEMORY {\n"
template_footer="}\n"
template_line="%20s : ORIGIN = %#10x, LENGTH = %u\n"


####
## helper functions
####

# \brief	check overlap of two memory regions
#
# \param	1	base address of first region
# \param	2	size of first region
# \param	3	base address of second region
# \param	4	size of second region
#
# \return	echo 0 if regions do not overlap
#			echo 1 if regions overlap
function overlap(){
	base0=$1
	end0=$(expr $1 + $2 - 1)
	base1=$3
	end1=$(expr $3 + $4 - 1)

	if [ ${base1} -gt ${end0} -o ${base0} -gt ${end1} ];then
		echo 0
	else
		echo 1
	fi
}


####
## main
####

printf "generating avr memory layout linker script \"%s\"\n" ${ofile}

## consitency checks
# check overlap of kernel and application flash
if [ $(overlap ${kernel_flash_base} ${kernel_flash_size} ${app_flash_base} ${app_flash_size}) -eq 1 ];then
	printf "\terror: kernel and application flash regions overlap, check config\n\n"
	printf "\t%20s %10s %10s %10s\n" "section" "base" "end" "size"
	printf "\t%20s %#10x %#10x %10u\n" "kernel flash" ${kernel_flash_base} $(expr ${kernel_flash_base} +  ${kernel_flash_size} - 1) ${kernel_flash_size}
	printf "\t%20s %#10x %#10x %10u\n\n" "application flash" ${app_flash_base} $(expr ${app_flash_base} +  ${app_flash_size} - 1) ${app_flash_size}

	exit 1
fi

# check overlap of kernel and application data
if [ $(overlap ${kernel_data_base} ${kernel_data_size} ${app_data_base} ${app_data_size}) -eq 1 ];then
	printf "\terror: kernel and application data regions overlap, check config\n\n"
	printf "\t%20s %10s %10s %10s\n" "section" "base" "end" "size"
	printf "\t%20s %#10x %#10x %10u\n" "kernel data" ${kernel_data_base} $(expr ${kernel_data_base} +  ${kernel_data_size} - 1) ${kernel_data_size}
	printf "\t%20s %#10x %#10x %10u\n\n" "application data" ${app_data_base} $(expr ${app_data_base} +  ${app_data_size} - 1) ${app_data_size}

	exit 1
fi

## generat linker script
mkdir -p $(dirname ${ofile})

printf "${template_header}" > ${ofile}
printf "${template_line}" ${kernel_flash_name} ${kernel_flash_base} ${kernel_flash_size} >> ${ofile}
printf "${template_line}" ${app_flash_name} ${app_flash_base} ${app_flash_size} >> ${ofile}
printf "${template_line}" ${kernel_data_name} ${kernel_data_base} ${kernel_data_size} >> ${ofile}
printf "${template_line}" ${app_data_name} ${app_data_base} ${app_data_size} >> ${ofile}
printf "${template_footer}" >> ${ofile}
