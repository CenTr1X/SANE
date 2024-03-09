#!/bin/bash

docker exec -d client bash -c "cd /home && ./config_multiapp $1"
