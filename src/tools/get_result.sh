#!/bin/bash

mkdir ../result/$1
exit_status=$?

if [ $exit_status -eq 0 ]; then
    if [ $2 = "d" ]; then
        echo 123
        docker cp gateway:/home/result_unit0.txt ../result/$1/result_unit0.txt
        docker cp body_dcu:/home/result_unit1.txt ../result/$1/result_unit1.txt
        docker cp body_dcu:/home/result_unit_sd1.txt ../result/$1/result_unit_sd1.txt
        docker cp powertrain_dcu:/home/result_unit2.txt ../result/$1/result_unit2.txt
        docker cp adas_dcu:/home/result_unit3.txt ../result/$1/result_unit3.txt
        docker cp adas_dcu:/home/result_unit_sd3.txt ../result/$1/result_unit_sd3.txt
        docker cp passive_safety_dcu:/home/result_unit4.txt ../result/$1/result_unit4.txt
        docker cp lidar:/home/result_unit_sd_sensor.txt ../result/$1/result_unit_sd_sensor.txt
        docker cp monitor:/home/result_unit_sd_monitor.txt ../result/$1/result_unit_sd_monitor.txt
    else
        docker cp gateway:/home/result_unit0.txt ../result/$1/result_unit0.txt
        docker cp lidar:/home/result_unit1.txt ../result/$1/result_unit1.txt
        docker cp monitor:/home/result_unit2.txt ../result/$1/result_unit2.txt
        docker cp front_left_zcu:/home/result_unit3.txt ../result/$1/result_unit3.txt
        docker cp front_right_zcu:/home/result_unit4.txt ../result/$1/result_unit4.txt
        docker cp rear_left_zcu:/home/result_unit5.txt ../result/$1/result_unit5.txt
        docker cp rear_right_zcu:/home/result_unit6.txt ../result/$1/result_unit6.txt
    fi
else
    echo "mkdir failed with exit status: $exit_status"
fi