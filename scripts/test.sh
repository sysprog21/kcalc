#!/bin/bash

CALC_DEV=/dev/calc
CALC_MOD=calc.ko

test_op() {
    local expression=$1 
    echo "Testing " ${expression} "..."
    echo -n ${expression}'\0' > $CALC_DEV
    cat $CALC_DEV
}    

if [ "$EUID" -eq 0 ]
  then echo "Don't run this script as root"
  exit
fi

sudo rmmod -f calc 2>/dev/null

modinfo $CALC_MOD || exit 1
sudo insmod calc.ko
sudo chmod 0666 $CALC_DEV
echo

# multiply
test_op '6*7'

# add
test_op '1980+1'

# sub
test_op '2019-1'

# testing div
test_op '42/6'

sudo rmmod calc
