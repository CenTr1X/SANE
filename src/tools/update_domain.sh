#!/bin/bash

docker cp ../build/domain/image.elf body_dcu:/home/dds.elf
docker cp ../build/domain/image.elf powertrain_dcu:/home/dds.elf
docker cp ../build/domain/image.elf adas_dcu:/home/dds.elf
docker cp ../build/domain/image.elf passive_safety_dcu:/home/dds.elf

docker cp ../build/dcu_s/image.elf adas_dcu:/home/sd_server.elf
docker cp ../build/dcu_c/image.elf body_dcu:/home/sd_server.elf
docker cp ../build/sensor/image.elf lidar:/home/
docker cp ../build/monitor/image.elf monitor:/home/