#!/bin/bash

docker exec -d lidar bash -c "cd /home && ./config.exp | tee out.txt"
docker exec -d monitor bash -c "cd /home && ./config.exp | tee out.txt"

docker exec -d body_dcu bash -c "cd /home && ./config_measurement.exp 1 | tee out.txt"
docker exec -d powertrain_dcu bash -c "cd /home && ./config_measurement.exp 2 | tee out.txt"
docker exec -d adas_dcu bash -c "cd /home && ./config_measurement.exp 3 | tee out.txt"
docker exec -d passive_safety_dcu bash -c "cd /home && ./config_measurement.exp 4 | tee out.txt"


