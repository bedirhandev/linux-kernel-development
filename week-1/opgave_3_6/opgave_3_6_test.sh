#!/bin/bash

sudo insmod opgave_3_6.ko
lsmod | grep opgave_3_6
sudo mknod /dev/opgave_3_6 c 243 0 -m 777
echo "hallo" > /dev/opgave_3_6
cat /dev/opgave_3_6
sudo rmmod opgave_3_6
sudo rm /dev/opgave_3_6
