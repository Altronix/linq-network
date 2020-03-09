FROM ubuntu

RUN apt-get -y update && \
	    apt-get install -y \
	    build-essential \
	    libtool \
	    autoconf \
	    automake \
	    git \
	    cmake \
	    uuid-dev \
	    libssl-dev \
	    clang 

COPY ./linq_network /tmp/linq_network
COPY ./linq_database /tmp/linq_database
COPY ./linq_daemon /tmp/linq_daemon
COPY ./linq_http /tmp/linq_http
COPY ./linq_utils /tmp/linq_utils
COPY ./scripts /tmp/scripts
COPY ./cmake /tmp/cmake
COPY ./CMakeLists.txt /tmp/CMakeLists.txt
COPY ./external /tmp/external
COPY ./docker/start.sh /opt/linq-network/bin/start.sh

WORKDIR /tmp

RUN \
	    mkdir -p build && \
	    cd build && \
	    cmake -DCMAKE_INSTALL_PREFIX=/opt/linqd \
		  -DUSE_SYSTEM_OPENSSL=ON \
		  -DENABLE_TESTING=OFF \
		  -DWITH_CPP_BINDING=OFF \
		  .. && \
	    make && \
	    make install

EXPOSE 8000 8001 33455

ENTRYPOINT ["/opt/linq-network/bin/start.sh"]
