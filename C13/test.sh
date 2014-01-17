#!/bin/bash

g++ main.cpp -o prod_cons ||{
	echo Compilation failed.
	exit 2
}

./prod_cons  || {
	echo Execution failed.
	exit 2
}

diff 1.out 2.out || {
	echo Consumers outputs differ.
	exit 2
}

diff 1.out pr.out || {
	echo Consumers and producer outputs differ.
	exit 2
}

echo OK

rm 1.out 2.out pr.out prod_cons

