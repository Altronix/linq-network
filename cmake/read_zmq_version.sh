#!/bin/sh
#
# This script extracts the 0MQ version from include/zmq.h, which is the master
# location for this information.
#
if [ ! -f $1 ]; then
    echo "version.sh: error: $1 does not exist" 1>&2
    exit 1
fi
MAJOR=`egrep '^#define +ZMQ_VERSION_MAJOR +[0-9]+$' $1`
MINOR=`egrep '^#define +ZMQ_VERSION_MINOR +[0-9]+$' $1`
PATCH=`egrep '^#define +ZMQ_VERSION_PATCH +[0-9]+$' $1`
if [ -z "$MAJOR" -o -z "$MINOR" -o -z "$PATCH" ]; then
    echo "version.sh: error: could not extract version from $1" 1>&2
    exit 1
fi
MAJOR=`echo $MAJOR | awk '{ print $3 }'`
MINOR=`echo $MINOR | awk '{ print $3 }'`
PATCH=`echo $PATCH | awk '{ print $3 }'`
echo $MAJOR | tr -d '\n'
echo _ | tr -d '\n'
echo $MINOR | tr -d '\n'
echo _ | tr -d '\n'
echo $PATCH | tr -d '\n'
