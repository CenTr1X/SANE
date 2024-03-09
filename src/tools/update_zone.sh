#!/bin/bash

docker cp ../build/zone/image.elf lidar:/home/dds.elf
docker cp ../build/zone/image.elf monitor:/home/dds.elf
docker cp ../build/zone/image.elf front_left_zcu:/home/dds.elf
docker cp ../build/zone/image.elf front_right_zcu:/home/dds.elf
docker cp ../build/zone/image.elf rear_left_zcu:/home/dds.elf
docker cp ../build/zone/image.elf rear_right_zcu:/home/dds.elf
