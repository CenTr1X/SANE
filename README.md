# SANE

SANE is an integrated in-vehicle network simulation framework. This repository includes all the code for the framework and the implementation of the scenarios presented in our paper.

## Prerequisites

### Running in Physical Machine

1. qemu-system-arm version 8.2
2. Make
3. gcc

### Running in Docker

1. Docker
2. Docker Compose

### Running tc

1. A system installed Netem mod

## Usage

### Running in Docker

We have uploaded the pre-built Docker images to Docker Hub. You just need to use `docker-compose` to run them.

```
docker-compose -f compose/docker-compose-<arch>.yml up -d
```

Note: The computer image is quite large, so it will take some time to download.

After all the containers have been started, you can run the script to launch all the QEMU virtual machines except for `linux-qemu`.

```
sh tools/run_<arch>.sh
```

The reason for separating the startup of `linux-qemu` and other QEMU virtual machines is because Linux has a slower boot time, which may cause the discovery phase to fail. Therefore, it is recommended to manually start QEMU in advance after entering the relevant container. You can run the following command in the container:

```
cd home && ./config_gateway.exp
```

After all message exchanges are completed, you can execute the command to shut down QEMU. However, for `linux-qemu`, you still need to manually execute Ctrl+C and the `poweroff` command to shut it down.

```
sh tools/stop_qemu.sh
```

Then, you can use a Python script to retrieve and analyze the data. Due to the large amount of output, it is recommended to redirect the output to a file.

```
sh tools/get_result.sh <dirname> d #used for domain-based
sh tools/get_result.sh <dirname> z #used for zone-based
sh tools/get_result_multiapp.sh <dirname> #used for multiapp
python3 analyser/resolve_data_<arch>.py <dirname> > output
```

### Running in Physical Machine

If you want to run it in the physical machine or modify the image, you should build the image manually. 

```
cd build && make -e type=<type>
```

Each `type` will compile a specific image, and the valid values for `type` and their corresponding images are as follows:

- sensor: used in `sensor` image
- dcu_s: used in  `dcu` image, implementing SOME/IP server function
- dcu_c: used in `dcu` image, implementing SOME/IP client function
- monitor: used in `monitor` image
- domain: used in `dcu` image, implementing DDS function
- zone: used in `zcu` image, implementing DDS function
- multiapp: used in `client` image
- doip_gateway: not used in image

Then you can run the image:

```
sudo qemu-system-arm -M versatilepb -nographic -m 256 -kernel <dirname>/image.elf 
```

If you want to update the image used in docker, then run:

```
sh tools/update_domain.sh #update images used in domain-based
sh tools/update_zone.sh #update images used in zone-based
```

## Code Architecture

- analyser: Python scripts used for analyzing data
- app: test functions and header files of measurement
- build: for building image and saving intermediate files 
  - build/lib: contains static lib from cyclone-dds, built by https://github.com/CenTr1X/cyclonedds-arm_port
- compose: docker compose configuration files
- config: config for SOME/IP-SD and SOME/IP
- dds: header files of cyclonedds and dds application sources 
- drivers: drivers like NIC, UART implementations.
- exp: expect scripts
- fatfs, FreeRTOS-Plus-FAT: filesystem
- FreeRTOS-LTS-Kernel: FreeRTOS kernel
- lib: some useful functions
- linux_qemu: source code in linux-qemu
- lwip: lwip source code
- network: source code that implements network stack
- result: the directory saving measurement data
- shell: source code that implements interactive shell
- startup: source code used for bootstrap
- time: directory for storing time synchronization test files
- tools: convenient tools

