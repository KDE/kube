#!/bin/sh

echo "OK Your orders please"
while :
do
	read cmd
	echo "OK"
	[ "$cmd" = "BYE" ] && break
done
