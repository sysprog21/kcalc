#!/bin/bash

CALC_DEV=/dev/calc
CALC_MOD=calc.ko

test_op() {
    local expression=$1 
    echo "Testing " ${expression} "..."
    echo -ne ${expression}'\0' > $CALC_DEV
    ret=$(cat $CALC_DEV)

    # Transfer the self-defined representation to real number
    num=$(($ret >> 4))
    frac=$(($ret&15))
    neg=$((($frac & 8)>>3))
    
    [[ $neg -eq 1 ]] && frac=$((-((~$frac&15)+1)))

    echo "$num*(10^$frac)" | bc -l
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

test_op '1/3'

test_op '1/3*6+2/4'

test_op '(1/3)+(2/3)'

test_op '(2145%31)+23'

sudo rmmod calc
