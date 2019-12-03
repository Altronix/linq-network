#!/bin/bash

SOURCES="$(find \
	./bindings \
	./examples \
	./include \
	./src \
	./test \
	-name '*.c' -o -name '*.h' \
	-o -name '*.cpp' -o -name '*.hpp' \
	-o -name '*.rs')"
SOURCES=($SOURCES)

for(( i = 0; i < ${#SOURCES[@]} ; i++));do
	if ! grep -q Copyright ${SOURCES[$i]}
	then
		echo ${SOURCES[$i]} want copyright
		cat ./scripts/copyright.txt ${SOURCES[$i]} > ${SOURCES[$i]}.new
		mv ${SOURCES[$i]}.new ${SOURCES[$i]}
	fi
done
