#!/bin/bash

target=opgave_4_1
device=opgave_4_1
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
