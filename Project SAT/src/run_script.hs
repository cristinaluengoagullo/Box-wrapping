#!/bin/bash

TIME_LIMIT=5 # In secs.
BENCH_DIR=../../instances
EXE=./boxwrapping
CHECKER=./../../checker

#ulimit -t 5

for ifile in $BENCH_DIR/*.in; do
    ofile=$(basename $ifile .in)_CP.out
    echo ""
    echo "------ File $ifile ------"
    timeout 120 /usr/bin/time -f "%e" $EXE $ifile > ../out/$ofile
    if [ $? != 0 ]; then
	echo "Timed out!"
    else
	echo "OK!"
    fi
    $CHECKER $ifile ../out/$ofile
done

