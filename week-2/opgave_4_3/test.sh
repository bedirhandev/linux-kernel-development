#!/bin/bash

echo "hallo wereld" /dev/mychardev-0
cat < /dev/mychardev-0

echo "hallo wereld 1" /dev/mychardev-1
cat < /dev/mychardev-1

cat < /dev/mychardev-0
cat < /dev/mychardev-1
