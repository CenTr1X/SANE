#!/bin/bash

docker cp ../exp/config_measurement_zone.exp front_left_zcu:/home/config_measurement.exp
docker cp ../exp/config_measurement_zone.exp front_right_zcu:/home/config_measurement.exp
docker cp ../exp/config_measurement_zone.exp rear_left_zcu:/home/config_measurement.exp
docker cp ../exp/config_measurement_zone.exp rear_right_zcu:/home/config_measurement.exp

docker cp ../exp/config_measurement_zone.exp lidar:/home/config_measurement.exp
docker cp ../exp/config_measurement_zone.exp monitor:/home/config_measurement.exp