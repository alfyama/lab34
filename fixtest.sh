#!/bin/sh
i=1
while [ "${i}" -le 10 ]; do
	expected=$(cat test"${i}".out)
	actual=$(lli test"${i}"_fix.bc)

	if [ "$expected" != "$actual" ]; then
		echo "Difference in output of test ${i}!"
	else
		echo "Test ${i} passed."
	fi
    i=$((i + 1))
done


echo "\nDef tests\n"
i=1
while [ "${i}" -le 5 ]; do
	opt -enable-new-pm=0 -load ./p34.so -def-pass test"${i}".bc -o test"${i}"_def.bc 2> test"${i}"m.def
	expected=$(sort test"${i}"m.def)
	actual=$(sort test"${i}".def)

	if [ "$expected" != "$actual" ]; then
		echo "Difference in output of test ${i}!"
	else
		echo "Test ${i} passed."
	fi
    i=$((i + 1))
done
