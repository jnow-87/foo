#!/bin/bash

gperffile=$1
cfile=$2	# c++ source file
header=$3	# output file name

#
# check file extension
#

suffix=${cfile##*.}

if [[ "${suffix}" != "c" ]];then
	echo -e $0: input file \"${cfile}\" is no C source file
	exit 1
fi

#
# get data
#

# get name lookup function
lookup_name=$(grep lookup-function-name ${gperffile} | cut -d ' ' -f 3)
[ "${lookup_name}" == "" ] && lookup_name=in_word_set

# check if lookup tables are const
const_tables=
[ "$(grep %readonly-tables ${gperffile})" == "" ] || const_tables=const

# get return type of lookup function
if [ "$(grep %struct-type ${gperffile})" == "" ];then
	lookup_return=char
else
	lookup_return=$(grep -oe '^struct [^;]\+' ${gperffile})
fi

#
# write header file
#

# generate macro based on header file
macro=$(basename ${header} | tr a-z. A-Z_)

# print header
printf "#ifndef %s\n" "${macro}" > ${header}
printf "#define %s\n\n\n" "${macro}" >> ${header}

# extract included header files
grep -e '#include' ${cfile} >> ${header}
printf "\n\n" >> ${header}

# add prototype for lookup-function
printf "%s %s *%s(register const char *str, register size_t len);\n" "${const_tables}" "${lookup_return}" "${lookup_name}" >> ${header}

# print footer
printf "\n\n#endif // %s" "${macro}" >> ${header}
