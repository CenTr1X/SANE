#!/bin/bash

#date +%s%3N
docker exec -d gateway bash -c "date +%s%3N > time.txt" &
#date +%s%3N
docker exec -d body_dcu bash -c "date +%s%3N > time.txt" &
#date +%s%3N
docker exec -d powertrain_dcu bash -c "date +%s%3N > time.txt" &
#date +%s%3N
docker exec -d adas_dcu bash -c "date +%s%3N > time.txt" &
#date +%s%3N
docker exec -d passive_safety_dcu bash -c "date +%s%3N > time.txt" &
#date +%s%3N
docker exec -d lidar bash -c "date +%s%3N > time.txt"  &
#date +%s%3N
docker exec -d monitor bash -c "date +%s%3N > time.txt"  &
#date +%s%3N

sleep 1

mkdir time
docker cp gateway_1:/time.txt ./time/1.txt
docker cp body_dcu_1:/time.txt ./time/2.txt
docker cp powertrain_dcu_1:/time.txt ./time/3.txt
        
docker cp adas_dcu_1:/time.txt ./time/4.txt
docker cp passive_safety_dcu_1:/time.txt ./time/5.txt
docker cp lidar_1:/time.txt ./time/6.txt
docker cp monitor_1:/time.txt ./time/7.txt

cat ./time/1.txt
cat ./time/2.txt
cat ./time/3.txt
cat ./time/4.txt
cat ./time/5.txt
cat ./time/6.txt
cat ./time/7.txt