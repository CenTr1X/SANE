make clean -e type=dds_app
make -e type=dds_app 
sudo qemu-system-arm -M versatilepb -nographic   -m 256 -kernel dds_app/image.elf -nic socket,mcast=230.0.0.1:6666 