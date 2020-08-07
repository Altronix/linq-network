#!/bin/bash

if [[ -z "${ENABLE_TESTING}" ]]; then ENABLE_TESTING=OFF; fi
if [[ -z "${BUILD_DEPENDENCIES}" ]]; then BUILD_DEPENDENCIES=OFF; fi
if [[ -z "${BUILD_LINQD}" ]]; then BUILD_LINQD=OFF; fi
if [[ -z "${BUILD_USBD}" ]]; then BUILD_USBD=OFF; fi
if [[ -z "${BUILD_USBH}" ]]; then BUILD_USBH=OFF; fi

echo "ENABLE_TESTING            : ${ENABLE_TESTING}"
echo "BUILD_DEPENDENCIES        : ${BUILD_DEPENDENCIES}"
echo "BUILD_LINQD               : ${BUILD_LINQD}"
echo "BUILD_USBD                : ${BUILD_USBD}"
echo "BUILD_USBH                : ${BUILD_USBH}"

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
		 	-DBUILD_USBD=$BUILD_USBD \
		 	-DBUILD_USBH=$BUILD_USBH \
		 	-DBUILD_DEPENDENCIES=$BUILD_DEPENDENCIES
		 
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
		 	-DBUILD_USBD=$BUILD_USBD \
		 	-DBUILD_USBH=$BUILD_USBH \
		 	-DBUILD_DEPENDENCIES=$BUILD_DEPENDENCIES
		 
		cmake --build . --target install
	) || exit 1
else
	(
		cmake \
			-DCMAKE_INSTALL_PREFIX=./build/install \
		 	-DENABLE_TESTING=$ENABLE_TESTING \
		 	-DBUILD_LINQD=$BUILD_LINQD \
		 	-DBUILD_USBD=$BUILD_USBD \
		 	-DBUILD_USBH=$BUILD_USBH \
		 	-DBUILD_DEPENDENCIES=$BUILD_DEPENDENCIES
		 
		cmake --build . --target install
	) || exit 1
fi
 
 if [ "$ENABLE_TESTING" = "ON" ]; then
 	ctest -T memcheck &&
 		make test-coverage &&
 		bash <(curl -s https://codecov.io/bash) -f test-coverage.info;
 fi
