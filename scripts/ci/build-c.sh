#!/bin/bash

cmake \
	-DCMAKE_INSTALL_PREFIX=./build/install \
	-DENABLE_TESTING=$ENABLE_TESTING \
	-DBUILD_EXAMPLES=$BUILD_EXAMPLES \
	-DWITH_SQLITE=$WITH_SQLITE \
	-USE_SYSTEM_ZMQ=$USE_SYSTEM_ZMQ \
	-USE_SYSTEM_JSMN_WEB_TOKENS=$USE_SYSTEM_JSMN_WEB_TOKENS \
	-USE_SYSTEM_OPENSSL=$USE_SYSTEM_OPENSSL

cmake --build . --target install

if [ "$ENABLE_TESTING" = "ON" ]; then
	ctest -T memcheck &&
		make test-coverage &&
		bash <(curl -s https://codecov.io/bash) -f test-coverage.info;
fi
