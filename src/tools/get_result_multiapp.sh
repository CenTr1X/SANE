#!/bin/bash

mkdir ../result/$1
exit_status=$?

if [ $exit_status -eq 0 ]; then
    docker cp client:/home/result_unit1.txt ../result/$1/result_unit1.txt
    docker cp server:/home/result_unit0.txt ../result/$1/result_unit0.txt
else
    echo "mkdir failed with exit status: $exit_status"
fi