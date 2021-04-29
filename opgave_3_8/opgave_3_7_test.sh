#!/bin/bash

target=opgave_3_8
device=opgave_3_8
driver=${target}.ko
syslog=/var/log/syslog
major_number=243

test() {
    local success;
    echo $1
    sudo insmod ${driver} $2 $3
    success=`lsmod | grep $target`
    if [ -n "$success" ]; # check for non-null/non-zero string variable
    then
        echo "$driver module is registered"
    else
        echo "$driver module is not registered"
    fi

    mknod /dev/${device} c ${major_number} 0
}

test "test executed with default parameters"
test "test executed with parameters" dvr_param1=1 dvr_param2=2
test "test executed with names" dvr_param1=3 dvr_param2=4