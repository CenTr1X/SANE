#!/bin/bash

docker cp ../exp/config_measurement_domain.exp body_dcu:/home/config_measurement.exp
docker cp ../exp/config_measurement_domain.exp powertrain_dcu:/home/config_measurement.exp
docker cp ../exp/config_measurement_domain.exp adas_dcu:/home/config_measurement.exp
docker cp ../exp/config_measurement_domain.exp passive_safety_dcu:/home/config_measurement.exp

docker cp ../exp/config_sensor.exp lidar:/home/config.exp
docker cp ../exp/config_monitor.exp monitor:/home/config.exp