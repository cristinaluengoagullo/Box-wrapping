#!/bin/bash

TIME_LIMIT=120 # In secs.
BENCH_DIR=../instances
CHECKER=../checker
EXE=./boxwrapping

for ifile in $BENCH_DIR/*.in; do
    ofile=$(basename $ifile .in)_LP.out
    echo ""
    echo "------ File $ifile ------"
    timeout 120s $EXE $ifile > ../out/$ofile
    if [ $? != 0 ]; then
	echo "Timed out!"
    else
	echo "OK!"
    fi
    $CHECKER $ifile ../out/$ofile
done
