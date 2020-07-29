#!/bin/bash

if [[ -z "${ENABLE_TESTING}" ]]; then ENABLE_TESTING=OFF; fi
if [[ -z "${USE_SYSTEM_ZMQ}" ]]; then USE_SYSTEM_ZMQ=OFF; fi
if [[ -z "${USE_SYSTEM_JSMN_WEB_TOKENS}" ]]; then USE_SYSTEM_JSMN_WEB_TOKENS=OFF; fi
if [[ -z "${USE_SYSTEM_OPENSSL}" ]]; then USE_SYSTEM_OPENSSL=OFF; fi
if [[ -z "${BUILD_LINQD}" ]]; then BUILD_LINQD=OFF; fi

echo "ENABLE_TESTING            : ${ENABLE_TESTING}"
echo "USE_SYSTEM_ZMQ            : ${USE_SYSTEM_ZMQ}"
echo "USE_SYSTEM_JSMN_WEB_TOKENS: ${USE_SYSTEM_JSMN_WEB_TOKENS}"
echo "USE_SYSTEM_OPENSSL        : ${USE_SYSTEM_OPENSSL}"
echo "BUILD_LINQD               : ${BUILD_LINQD}"

# If a generator is provided we are assuming a windows build for now
# Travis wants CMAKE_GENERATOR_PLATFORM to find correct libraries
if [ "$GENERATOR" == "Visual Studio 15 2017 Win64" ]; then
	echo "GENERATOR                 : ${GENERATOR}"
	echo "GENERATOR_PLATFORM        : x64"
	(
		cmake \
			-G "$GENERATOR" \
			-DCMAKE_INSTALL_PREFIX=./build/install \
		 	-DENABLE_TESTING=$ENABLE_TESTING \
		 	-DBUILD_LINQD=$BUILD_LINQD \
		 	-DUSE_SYSTEM_ZMQ=$USE_SYSTEM_ZMQ \
		 	-DUSE_SYSTEM_JSMN_WEB_TOKENS=$USE_SYSTEM_JSMN_WEB_TOKENS \
		 	-DUSE_SYSTEM_OPENSSL=$USE_SYSTEM_OPENSSL
		 
		cmake --build . --target install
	) || exit 1
elif [ "$GENERATOR" == "Visual Studio 16 2019" ]; then
	echo "GENERATOR                 : ${GENERATOR}"
	echo "GENERATOR_PLATFORM        : x64"
	(
		cmake \
			-G "$GENERATOR" \
			-A "x64" \
			-DCMAKE_INSTALL_PREFIX=./build/install \
		 	-DENABLE_TESTING=$ENABLE_TESTING \
		 	-DBUILD_LINQD=$BUILD_LINQD \
		 	-DUSE_SYSTEM_ZMQ=$USE_SYSTEM_ZMQ \
		 	-DUSE_SYSTEM_JSMN_WEB_TOKENS=$USE_SYSTEM_JSMN_WEB_TOKENS \
		 	-DUSE_SYSTEM_OPENSSL=$USE_SYSTEM_OPENSSL
		 
		cmake --build . --target install
	) || exit 1
else
	(
		cmake \
			-DCMAKE_INSTALL_PREFIX=./build/install \
		 	-DENABLE_TESTING=$ENABLE_TESTING \
		 	-DBUILD_LINQD=$BUILD_LINQD \
		 	-DUSE_SYSTEM_ZMQ=$USE_SYSTEM_ZMQ \
		 	-DUSE_SYSTEM_JSMN_WEB_TOKENS=$USE_SYSTEM_JSMN_WEB_TOKENS \
		 	-DUSE_SYSTEM_OPENSSL=$USE_SYSTEM_OPENSSL
		 
		cmake --build . --target install
	) || exit 1
fi
 
 if [ "$ENABLE_TESTING" = "ON" ]; then
 	ctest -T memcheck &&
 		make test-coverage &&
 		bash <(curl -s https://codecov.io/bash) -f test-coverage.info;
 fi
