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
