FROM ubuntu

RUN apt-get -y update && \
	    apt-get install -y \
	    build-essential \
	    libtool \
	    autoconf \
	    automake \
	    cmake \
	    libssl-dev \
	    clang 

COPY ./linq_network /tmp/linq_network
COPY ./examples /tmp/examples
COPY ./scripts /tmp/scripts
COPY ./include /tmp/include
COPY ./cmake /tmp/cmake
COPY ./CMakeLists.txt /tmp/CMakeLists.txt
COPY ./external /tmp/external

WORKDIR /tmp

RUN \
	    mkdir -p build && \
	    cd build && \
	    cmake -DCMAKE_INSTALL_PREFIX=/opt/linq-network \
	    	  -DWITH_SQLITE=ON \
		  -DUSE_SYSTEM_OPENSSL=ON \
		  -DBUILD_EXAMPLES=ON \
		  -DENABLE_TESTING=OFF \
		  .. && \
	    make && \
	    make install

EXPOSE 8000 8001

ENTRYPOINT ["./docker/start.sh"]
