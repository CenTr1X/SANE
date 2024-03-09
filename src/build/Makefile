# Copyright 2013, Jernej Kovacic
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#
# Type "make help" for more details.
#

TOOLCHAIN = arm-none-eabi-
CC = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AS = $(TOOLCHAIN)as
LD = $(TOOLCHAIN)ld
OBJCOPY = $(TOOLCHAIN)objcopy
AR = $(TOOLCHAIN)ar

BUILD_DIR = .

# GCC flags
INCLUDEFLAG = -I
CPUFLAG = -mcpu=arm926ej-s  
#CPUFLAG = -mcpu=cortex-a9
#WFLAG = -Wall -Wextra -Werror
WFLAG = 
# CFLAGS = $(CPUFLAG) $(WFLAG) --specs=nano.specs
CFLAGS = -std=gnu99 -g -O0 $(CPUFLAG) $(WFLAG) -mtp=auto

# Additional C compiler flags to produce debugging symbols
DEB_FLAG = -g -DDEBUG

# Compiler/target path in FreeRTOS/Source/portable
PORT_COMP_TARG = GCC/ARM926EJ-S/
#PORT_COMP_TARG = GCC/ARM_CA9

# use these on the $(LD) line, as needed:
LIBM = ${shell ${CC} ${CFLAGS} --print-file-name=libm.a}
LIBC = ${shell ${CC} ${CFLAGS} --print-file-name=libc.a}
LIBGCC = ${shell ${CC} ${CFLAGS} --print-libgcc-file-name}
#LIBGCC = /usr/lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/lib/libgcc.a
#LIBSEMI = /usr/lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/lib/libsemihost.a
LIBDDS = ./lib/libddsc.a 

# Intermediate directory for all *.o and other files:
SRCDIR = ..

# FreeRTOS source base directory
FREERTOS_SRC = $(SRCDIR)/FreeRTOS-LTS-Kernel/

# Directory with memory management source files
FREERTOS_MEMMANG_SRC = $(FREERTOS_SRC)portable/MemMang/

# Directory with platform specific source files
FREERTOS_PORT_SRC = $(FREERTOS_SRC)portable/$(PORT_COMP_TARG)

# file system src
FREERTOS_FAT_SRC = $(SRCDIR)/FreeRTOS-Plus-FAT/
FREERTOS_FAT_PORTABLE_SRC = $(SRCDIR)/FreeRTOS-Plus-FAT/portable/common/

# Directory with HW drivers' source files
DRIVERS_SRC = $(SRCDIR)/drivers/

# Directory with startup source (and header) files. Also linker script
STARTUP_SRC = $(SRCDIR)/startup/

# Directory with startup source (and header) files. Also linker script
FATFS_SRC = $(SRCDIR)/fatfs/

# Directory with demo specific source (and header) files
APP_SRC = $(SRCDIR)/app/

# shell
SHELL_SRC = $(SRCDIR)/shell

# SoAd
SOAD_SRC = $(SRCDIR)/network/SoAd/

#Tcp/Ip
TCPIP_SRC = $(SRCDIR)/network/TcpIp/

#SOME/IP
SOMEIP_SRC = $(SRCDIR)/network/SomeIp/

#DOIP
DOIP_SRC = $(SRCDIR)/network/DoIP/

#DDS
DDS_SRC = $(SRCDIR)/dds/src

DETECTION_SRC = $(DDS_SRC)/tire_pressure_detection

# Directory with demo specific source (and header) files
LIB_SRC = $(SRCDIR)/lib/
LIB_READLINE_SRC = $(SRCDIR)/lib/readline

# Object files to be linked into an application
# Due to a large number, the .o files are arranged into logical groups:

FREERTOS_OBJS = queue.o list.o tasks.o
# The following o. files are only necessary if
# certain options are enabled in FreeRTOSConfig.h
FREERTOS_OBJS += timers.o
FREERTOS_OBJS += event_groups.o
# FREERTOS_OBJS += croutine.o
# FREERTOS_OBJS += stream_buffer.o

# Only one memory management .o file must be uncommented!
#FREERTOS_MEMMANG_OBJS = heap_1.o
#FREERTOS_MEMMANG_OBJS = heap_2.o
FREERTOS_MEMMANG_OBJS = heap_3.o
#FREERTOS_MEMMANG_OBJS = heap_4.o
#FREERTOS_MEMMANG_OBJS = heap_5.o

#FREERTOS_PORT_OBJS = port.o portISR.o
FREERTOS_PORT_OBJS = port.o portISR.o

FREERTOS_FAT_OBJS = ff_crc.o ff_dir.o ff_error.o ff_fat.o ff_file.o ff_format.o ff_ioman.o 
FREERTOS_FAT_OBJS += ff_locking.o ff_memory.o ff_stdio.o ff_string.o ff_sys.o ff_time.o
FREERTOS_FAT_PORTABLE_OBJS = ff_ramdisk.o

LWIP_OBJS = dhcp.o acd.o err.o tcpip.o sys_arch.o if_api.o \
			sockets.o api_lib.o api_msg.o netbuf.o \
              init.o def.o dns.o inet_chksum.o ip.o mem.o memp.o netif.o \
              pbuf.o raw.o stats.o sys.o altcp.o altcp_alloc.o altcp_tcp.o \
              tcp.o tcp_in.o tcp_out.o timeouts.o udp.o icmp.o ip4.o \
              ip4_addr.o ip4_frag.o ethernet.o etharp.o igmp.o

LWIP_INCS = -I $(SRCDIR)/lwip/src/include/

INC_NETWORK = -I $(SRCDIR)/network/include

STARTUP_ASM_OBJ = startup.o

STARTUP_OBJS = main.o 

FATFS_OBJS = syscalls.o ramdisk.o untar.o

DRIVERS_OBJS = timer.o pic.o sic.o uart.o bsp.o ethernetif.o lan91c.o console.o arpfilter.o vlan.o

APP_OBJS = demotasks.o disktest.o

SHELL_OBJS = shell.o command.o ifconfig.o arp.o ping.o vconf.o net.o iperfapp.o cmdlwiperf.o lwiperf_dualproto.o

SOAD_OBJS = SoAd.o

TCPIP_OBJS = TcpIp.o

SOMEIP_OBJS = SomeIp.o SomeIpSd.o

DOIP_OBJS = DoIP.o

DDS_OBJS = 

CONFIG_OBJS = SoAd_Cfg.o 

# Definition of the linker script and final targets
LINKER_SCRIPT = $(addprefix $(STARTUP_SRC), qemu.ld)
ELF_IMAGE = image.elf
TARGET = image.bin

NETWORK_OBJS = $(SOAD_OBJS) $(TCPIP_OBJS)

ifeq ($(type), sensor)
	CONFIG_SRC = $(SRCDIR)/config/sensor
	CONFIG_OBJS += SomeIpSd_Cfg.o SomeIp_Callback.o SomeIp_Cfg.o
	TARGET = sensor.bin
	CFLAGS += -DUSE_SOMEIP
	NETWORK_OBJS += $(SOMEIP_OBJS)
	BUILD_DIR = $(type)
	SHELL_OBJS += rawtcpecho.o rawudpecho.o rawmultiecho.o
	IDL_OBJS = dds/HelloWorldData.o
else ifeq ($(type), dcu_s)
	CONFIG_SRC = $(SRCDIR)/config/dcu_s
	CONFIG_OBJS += SomeIpSd_Cfg.o SomeIp_Callback.o SomeIp_Cfg.o
	TARGET = dcu_s.bin
	CFLAGS += -DUSE_SOMEIP
	NETWORK_OBJS += $(SOMEIP_OBJS)
	BUILD_DIR = $(type)
	SHELL_OBJS += rawtcpecho.o rawudpecho.o rawmultiecho.o
	IDL_OBJS = dds/HelloWorldData.o
else ifeq ($(type), dcu_c)
	CONFIG_SRC = $(SRCDIR)/config/dcu_c
	CONFIG_OBJS += SomeIpSd_Cfg.o SomeIp_Callback.o SomeIp_Cfg.o
	TARGET = dcu_c.bin
	CFLAGS += -DUSE_SOMEIP
	NETWORK_OBJS += $(SOMEIP_OBJS)
	BUILD_DIR = $(type)
	SHELL_OBJS += rawtcpecho.o rawudpecho.o rawmultiecho.o
	IDL_OBJS = dds/HelloWorldData.o
else ifeq ($(type), monitor)
	CONFIG_SRC = $(SRCDIR)/config/monitor
	CONFIG_OBJS += SomeIpSd_Cfg.o SomeIp_Callback.o SomeIp_Cfg.o
	TARGET = monitor.bin
	CFLAGS += -DUSE_SOMEIP
	NETWORK_OBJS += $(SOMEIP_OBJS)
	BUILD_DIR = $(type)
	SHELL_OBJS += rawtcpecho.o rawudpecho.o rawmultiecho.o
	IDL_OBJS = dds/HelloWorldData.o
else ifeq ($(type), domain)
	CONFIG_SRC = 
	CONFIG_OBJS = 
	TARGET = dds_app.bin
	CFLAGS += -DUSE_DDS -DUSE_MEASUREMENT -DUSE_DOMAIN
	NETWORK_OBJS = 
	DDS_OBJS += utils.o measurement_config_domain.o 
	BUILD_DIR = $(type)
	IDL_OBJS = dds/CommonData.o
	DDS_APP_SRC = $(DDS_SRC)/measurement
else ifeq ($(type), zone)
	CONFIG_SRC = 
	CONFIG_OBJS = 
	TARGET = dds_app.bin
	CFLAGS += -DUSE_DDS -DUSE_MEASUREMENT -DUSE_ZONE
	NETWORK_OBJS = 
	DDS_OBJS += utils.o measurement_config_zone.o 
	BUILD_DIR = $(type)
	IDL_OBJS = dds/CommonData.o dds/HelloWorldData.o
	DDS_APP_SRC = $(DDS_SRC)/measurement
else ifeq ($(type), multiapp)
	CONFIG_SRC = 
	CONFIG_OBJS = 
	TARGET = dds_app.bin
	CFLAGS += -DUSE_DDS -DUSE_MEASUREMENT -DUSE_ZONE -DUSE_MULTI
	NETWORK_OBJS = 
	DDS_OBJS += utils.o measurement_config_zone.o 
	BUILD_DIR = $(type)
	IDL_OBJS = dds/CommonData.o 
	DDS_APP_SRC = $(DDS_SRC)/multiapp
else ifeq ($(type), doip_gateway)
	CONFIG_SRC = $(SRCDIR)/config/DoIP_Gateway
	CONFIG_OBJS += DoIP_Cfg.o DoIP_Callback.o
	TARGET = doip_gateway.bin
	CFLAGS += -DUSE_DOIP
	NETWORK_OBJS += ${DOIP_OBJS}
	BUILD_DIR = $(type)
	SHELL_OBJS += doip_tester.o rawtcpecho.o rawudpecho.o rawmultiecho.o
else
	CONFIG_SRC = $(SRCDIR)/config/tcpecho
	SHELL_OBJS += tcpecho.o rawtcpecho.o rawudpecho.o rawmultiecho.o
endif

CFLAGS += -DSANE_DBG_ON=0xF0U
ifneq ($(debug), release)
#CFLAGS += -DSANE_DBG_ON=0xF0U
endif

# nostdlib.o must be commented out if standard lib is going to be linked!
#LIB_OBJS = nostdlib.o
LIB_OBJS += print.o receive.o printf.o debug.o readline.o history.o md5_copy.o

# All object files
TEMP_OBJS = $(STARTUP_ASM_OBJ) $(STARTUP_OBJS) $(FATFS_OBJS) $(FREERTOS_OBJS) $(FREERTOS_MEMMANG_OBJS) $(FREERTOS_PORT_OBJS) $(FREERTOS_FAT_OBJS) $(FREERTOS_FAT_PORTABLE_OBJS) $(DRIVERS_OBJS) $(APP_OBJS) $(LWIP_OBJS) $(LIB_OBJS) $(SHELL_OBJS) $(NETWORK_OBJS) $(CONFIG_OBJS) $(DDS_OBJS) tarfile.o
OBJS = $(foreach obj, $(TEMP_OBJS), $(addprefix $(BUILD_DIR)/, $(obj)))

# Include paths to be passed to $(CC) where necessary
INC_FREERTOS = -I $(FREERTOS_SRC)include/
INC_FREERTOS_FAT = -I $(FREERTOS_FAT_SRC)include/
INC_FREERTOS_FAT_PORTABLE = -I $(FREERTOS_FAT_SRC)portable/common
INC_DRIVERS = -I $(DRIVERS_SRC)
INC_LIB = -I $(LIB_SRC)
INC_LIB_READLINE = -I $(LIB_READLINE_SRC)
INC_SHELL = -I $(SHELL_SRC) -I $(SHELL_SRC)/iperf

ifeq ($(CONFIG_SRC),)
	INC_CONFIG =
else
	INC_CONFIG = -I $(CONFIG_SRC)
endif
INC_DDS = -I $(DDS_SRC)/../include

# Complete include flags to be passed to $(CC) where necessary
INC_FLAGS = -I $(STARTUP_SRC) -I $(APP_SRC) -I $(FATFS_SRC) -I $(FREERTOS_PORT_SRC) \
			$(INC_LIB) $(INC_FREERTOS) $(INC_FREERTOS_FAT) $(INC_FREERTOS_FAT_PORTABLE) $(LWIP_INCS) \
			$(INC_DRIVERS) $(INC_LIB_READLINE) $(INC_SHELL) $(INC_NETWORK) $(INC_CONFIG) $(INC_DDS) -I /usr/arm-none-eabi/include




# Dependency on HW specific settings
#DEP_BSP = $(INC_DRIVERS)bsp.h

#
# Make rules:
#

# printlibs::
#	echo $(LIBC)
#	echo $(LIBM)
#	echo $(LIBGCC)
#

vpath %.c $(SRCDIR)/lwip/src/ $(SRCDIR)/lwip/src/api/ $(SRCDIR)/lwip/src/core/ $(SRCDIR)/lwip/src/netif/ $(SRCDIR)/lwip/src/core/ipv4/ \
	$(FREERTOS_SRC) \
	$(FREERTOS_MEMMANG_SRC) \
	$(FREERTOS_PORT_SRC) \
	$(FREERTOS_FAT_SRC) \
	$(FREERTOS_FAT_PORTABLE_SRC) \
	$(DRIVERS_SRC) \
	$(STARTUP_SRC) \
	$(FATFS_SRC) \
	$(APP_SRC) \
	$(LIB_SRC) \
	$(SHELL_SRC) \
	$(SHELL_SRC)/iperf/ \
	$(LIB_READLINE_SRC) \
	$(SOAD_SRC) \
	$(TCPIP_SRC) \
	$(SOMEIP_SRC) \
	$(DOIP_SRC) \
	$(CONFIG_SRC) \
	$(DDS_SRC) \
	$(DDS_APP_SRC)

.PHONY: all clean run

all : $(BUILD_DIR)/$(TARGET)

%.o : %.c
	$(CC) -g -c -o $@ $(CFLAGS) $(INC_FLAGS) -o $@ $< 

$(BUILD_DIR)/%.o : $(STARTUP_SRC)/%.s
	$(AS) -g -c -o $@ $(CPUFLAG) $<

$(BUILD_DIR)/%.o : %.c
	$(CC) -g -c -o $@ $(CFLAGS) $(INC_FLAGS) -o $@ $<

rebuild : clean all

$(BUILD_DIR)/$(TARGET) : $(BUILD_DIR)/$(ELF_IMAGE)
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(ELF_IMAGE) $@

$(BUILD_DIR)/$(ELF_IMAGE) : $(OBJS) $(LINKER_SCRIPT)
#$(LD) -T $(LINKER_SCRIPT) $(OBJS) -o $@  $(LIBDDS) $(LIBC) $(LIBM) $(LIBGCC) 
	$(LD) -T $(LINKER_SCRIPT) $(OBJS) $(IDL_OBJS) -o $@  $(LIBDDS) $(LIBC) $(LIBM) $(LIBGCC) $(LIBSEMI) 
#/usr/lib/gcc/arm-none-eabi/10.3.1/../../../../arm-none-eabi/lib/libcrt0-minimal.a
#	$(LD) -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@  $(LIBDIR)/libc.a $(LIBDIR)/libm.a $(GCCLIBDIR)/libgcc.a 
#	$(LD) -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@  $(LIBDIR)/libc_nano.a 
#	$(LD) -nostdlib -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@ 

run : $(TARGET)
	qemu-system-arm -M versatilepb -nographic -m 128 -semihosting -kernel $< -net nic,model=smc91c111 -net tap,ifname=tap0,script=no,downscript=no

debug : $(TARGET)
	qemu-system-arm -M versatilepb -nographic -m 128 -semihosting -kernel $< -s -S -net nic,model=smc91c111 -net tap,ifname=tap0,script=no,downscript=no

#
# Tar file
#
$(BUILD_DIR)/tarfile.o :
	tar cf tarfile rootfs
	$(LD) -r --noinhibit-exec -o $(BUILD_DIR)/tarfile.o -b binary tarfile

# Cleanup directives:

clean_obj :
	$(RM) $(BUILD_DIR)/*.o 

clean_intermediate : clean_obj
	$(RM) $(BUILD_DIR)/*.elf
	$(RM) $(BUILD_DIR)/*.img

clean : clean_intermediate
	$(RM) $(BUILD_DIR)/*.bin
	$(RM) $(BUILD_DIR)/tarfile

# Short help instructions:

help :
	@echo
	@echo Valid targets:
	@echo - all: builds missing dependencies and creates the target image \'$(TARGET)\'.
	@echo - rebuild: rebuilds all dependencies and creates the target image \'$(TARGET)\'.
	@echo - debug: same as \'all\', also includes debugging symbols to \'$(ELF_IMAGE)\'.
	@echo - debug_rebuild: same as \'rebuild\', also includes debugging symbols to \'$(ELF_IMAGE)\'.
	@echo - clean_obj: deletes all object files, only keeps \'$(ELF_IMAGE)\' and \'$(TARGET)\'.
	@echo - clean_intermediate: deletes all intermediate binaries, only keeps the target image \'$(TARGET)\'.
	@echo - clean: deletes all intermediate binaries, incl. the target image \'$(TARGET)\'.
	@echo - help: displays these help instructions.
	@echo

show_config :
	@echo ${LIB_SRC}
	@echo ${CONFIG_OBJS}
	@echo $(CONFIG_SRC)
	@echo ${TEMP_OBJS}
	@echo $(OBJS)

.PHONY : all rebuild clean clean_obj clean_intermediate debug debug_rebuild _debug_flags help show_config