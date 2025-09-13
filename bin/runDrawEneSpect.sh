#! /bin/sh

INPUTFILE=$1
DETECTOR=$2

echo ${INPUTFILE}
COMMAND=${RDSW}/root_macros/drawEneSpect.cpp
root -l -b -q "${COMMAND}(\"${INPUTFILE}\",\"${DETECTOR}\")"

