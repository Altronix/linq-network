#!/bin/sh

cmake \
	-DCMAKE_INSTALL_PREFIX=./build/install \
	-DENABLE_TESTING=$ENABLE_TESTING \
	-DBUILD_EXAMPLES=$BUILD_EXAMPLES \
	-DWITH_SQLITE=$WITH_SQLITE

cmake --build . --target install

if [ $ENABLE_TESTING ]; then
	ctest -T memcheck &&
		make test-coverage &&
          	curl -s https://codecov.io/bash -f test-coverage.info;
fi

