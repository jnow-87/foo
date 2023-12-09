#!/bin/bash
#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#
#
# \brief	Find the given library in the compiler library path and
#			rename symbols according to the aliases defined in the
#			source code given on stdin
#			The provided source code is assumed to use the gcc alias
#			attribute to define a symbol mapping of the following
#			format:
#				[//] type <repl> .* __attribute__((alias("<sym-prefix><pat>")));
#
#			The script will then replace all occurences of <pat>
#			within the library with <repl>
#
# \param	target library	library to "create"
# \param	compiler		compiler to search for the library
# \param	symbol prefix	prefix to strip from the alias



#
# check parameter
#

lib_tgt=$1
compiler=$2
sym_prefix=$3

echo "generate ${lib_tgt}"

[ "${lib_tgt}" != "" ] || { echo "empty target library"; exit 1; }
[ "${compiler}" != "" ] || { echo "empty compiler"; exit 1; }
[ "${sym_prefix}" != "" ] || { echo "empty prefix"; exit 1; }


#
# try to find library
#

lib_name=$(basename ${lib_tgt})

compiler_dirs=$(${compiler} -print-search-dirs 2>&1 \
	| grep "programs\|libraries" \
	| cut -d "=" -f 2 \
	| sed -e 's/:/ /g'
)

lib=""

for dir in ${compiler_dirs}
do
	[ "${lib}" == "" -a -d ${dir} ] && lib=$(find ${dir} -name ${lib_name})
done

[ "${lib}" != "" ] || { echo "unable to locate ${lib_name}"; exit 1; }

#
# parse source file for aliases
#

# disabled wildcard expansion
set -f

pairs=""

# read aliases from stdin
while read -r line
do
	if [ "$(echo "$line" | grep "__alias[ (]\+.*[ )]\+")" == "" ];then
		continue
	fi

	# parse alias line according to the following pattern
	#	<comment>(optional)<type>[*]<pat>.*alias("<repl>").*
	alias=$(echo ${line} | sed -e "s:^[/ ]*[^ ]*[ \*]\+\([a-zA-Z0-9_-]\+\).*__alias[ (]\+\([^) ]*\)[ )]\+.*:\1=\2:")
	pat=$(echo ${alias} | cut -d '=' -f 2 | sed -e "s:\<${sym_prefix}::g")
	repl=$(echo ${alias} | cut -d '=' -f 1)

	if [ ! ${#pat} -eq ${#repl} ];then
		echo "length of \"${pat}\" and \"${repl}\" differ, pattern and replacement must have the same size"
		exit 1
	fi

	pairs+="${#pat} ${pat}=${repl}\n"
done

#
# copy library to target
#

echo "renaming symbols from ${lib}"
mkdir -p $(dirname ${lib_tgt})
cp ${lib} ${lib_tgt}

#
# rename symbols in library
#

# ensure longer symbols are renamed before shorter ones to avoid
# breaking the longer symbols, e.g. close vs. fclose
pairs=$(echo -e ${pairs} | sort -rh | cut -d " " -f 2-)

for pair in $(echo ${pairs});do
	pat=$(echo ${pair} | cut -d '=' -f 1)
	repl=$(echo ${pair} | cut -d '=' -f 2)

	sed -i -e  "s:${pat}:${repl}:g" ${lib_tgt}
done

exit 0
