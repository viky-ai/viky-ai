#!/usr/bin/env bash
#set -v

# rewrite simple lhp programme to template file

# /!\ launch with `env -i` to reset all variables

if [ $# -lt 2 ]
then
 echo "usage $0 \"variables definition\" input_file <output_file>"
 exit 1
fi

LHP_VARIABLES_SET=$1
LHP_VARIABLES_OUTPUT=$2
LHP_VARIABLES_TEMPLATE=$3
if [ -z "${LHP_VARIABLES_TEMPLATE}" ]
then
  LHP_VARIABLES_TEMPLATE=/dev/stdin
fi

# list variables already here
LHP_VARIABLES_BEFORE=`compgen -v`

# load variables
source /dev/stdin <<< "${LHP_VARIABLES_SET}"

# list all variables
LHP_VARIABLES_AFTER=`compgen -v`

# diff to find new variables
LHP_VARIABLES_PARSED=`comm <(echo "$LHP_VARIABLES_AFTER" | grep -v "LHP_VARIABLES_") <(echo "$LHP_VARIABLES_BEFORE" | grep -v "LHP_VARIABLES_") -3`

# reset ouput file
cat ${LHP_VARIABLES_TEMPLATE} > ${LHP_VARIABLES_OUTPUT}
for variable in $LHP_VARIABLES_PARSED; do
  value=`echo ${!variable}`

  # replace value
  sed -i "s|\[%${variable}%\]|${value}|g" ${LHP_VARIABLES_OUTPUT}
done
