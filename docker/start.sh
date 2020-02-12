#!/bin/bash

# Args from docker will arrive here

# Start the docker app
start_app(){
	echo "Starting app... $@"
	/opt/linq-network/bin/server "$@"
}

start_app $@
