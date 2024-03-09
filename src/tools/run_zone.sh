#!/bin/bash

docker exec -d lidar bash -c "cd /home && ./config_measurement.exp 1 | tee out.txt"
docker exec -d monitor bash -c "cd /home && ./config_measurement.exp 2 | tee out.txt"
docker exec -d front_left_zcu bash -c "cd /home && ./config_measurement.exp 3"
docker exec -d front_right_zcu bash -c "cd /home && ./config_measurement.exp 4"
docker exec -d rear_left_zcu bash -c "cd /home && ./config_measurement.exp 5"
docker exec -d rear_right_zcu bash -c "cd /home && ./config_measurement.exp 6"