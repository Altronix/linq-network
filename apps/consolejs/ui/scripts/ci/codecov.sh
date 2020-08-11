#!/bin/bash

export CODECOV_TOKEN="cd830e07-c7cd-4db3-b3af-84d8f6939479"
bash <(curl -s https://codecov.io/bash) -f ./coverage/lcov.info;
