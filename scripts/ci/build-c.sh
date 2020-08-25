#!/bin/bash

args=("-DCMAKE_INSTALL_PREFIX=./build/install")

if ! [[ -z "${ENABLE_TESTING}" ]]; then args+=("-DENABLE_TESTING=ON"); fi
if ! [[ -z "${BUILD_DEPENDENCIES}" ]]; then args+=("-DBUILD_DEPENDENCIES=ON"); fi
if ! [[ -z "${BUILD_LINQD}" ]]; then args+=("-DBUILD_LINQD=ON"); fi
if ! [[ -z "${BUILD_USBD}" ]]; then args+=("-DBUILD_USBD=ON"); fi
if ! [[ -z "${BUILD_USBH}" ]]; then args+=("-DBUILD_USBH=ON"); fi
echo "ENABLE_TESTING            : ${ENABLE_TESTING}"
echo "BUILD_DEPENDENCIES        : ${BUILD_DEPENDENCIES}"
echo "BUILD_LINQD               : ${BUILD_LINQD}"
echo "BUILD_USBD                : ${BUILD_USBD}"
echo "BUILD_USBH                : ${BUILD_USBH}"

# If a generator is provided we are assuming a windows build for now
# Travis wants CMAKE_GENERATOR_PLATFORM to find correct libraries
if [ "$GENERATOR" == "Visual Studio 15 2017 Win64" ]; then
	args+=("-G Visual Studio 15 2017 Win64")
	echo "GENERATOR                 : ${GENERATOR}"
	echo "GENERATOR_PLATFORM        : x64"
	echo "ARGS                      : ${args[@]}"
	(
		cmake ${args[@]}
		cmake --build . --target install
	) || exit 1
elif [ "$GENERATOR" == "Visual Studio 16 2019" ]; then
	args+=("-G Visual Studio 16 2019")
	args+=("-A x64")
	echo "GENERATOR                 : ${GENERATOR}"
	echo "GENERATOR_PLATFORM        : x64"
	echo "ARGS                      : ${args[@]}"
	(
		cmake ${args[@]}
		cmake --build . --target install
	) || exit 1
else
	(
		cmake ${args[@]}
		cmake --build . --target install
	) || exit 1
fi
 
 if [ "$ENABLE_TESTING" = "ON" ]; then
 	ctest -T memcheck &&
 		make test-coverage &&
 		bash <(curl -s https://codecov.io/bash) -f test-coverage.info;
 fi
