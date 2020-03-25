#!/bin/bash

CALC_DEV=/dev/calc
CALC_MOD=calc.ko

test_op() {
    local expression=$1 
    local NAN_INT=31
    local INF_INT=47
    echo "Testing " ${expression} "..."
    echo -ne ${expression}'\0' > $CALC_DEV
    ret=$(cat $CALC_DEV)

    # Transfer the self-defined representation to real number
    num=$(($ret >> 4))
    frac=$(($ret&15))
    neg=$((($frac & 8)>>3))
    
    [[ $neg -eq 1 ]] && frac=$((-((~$frac&15)+1)))

    if [ "$ret" -eq "$NAN_INT" ]
    then
        echo "NAN_INT"
    elif [ "$ret" -eq "$INF_INT" ]
    then
        echo "INF_INT"
    else
      echo "$num*(10^$frac)" | bc -l
    fi
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

# div
test_op '42/6'
test_op '1/3'
test_op '1/3*6+2/4'
test_op '(1/3)+(2/3)'
test_op '(2145%31)+23'
test_op '0/0' # should be NAN_INT

# binary
test_op '(3%0)|0' # should be 0
test_op '1+2<<3' # should be (1 + 2) << 3 = 24
test_op '123&42' # should be 42
test_op '123^42' # should be 81

# parens
test_op '(((3)))*(1+(2))' # should be 9

# assign
test_op 'x=5, x=(x!=0)' # should be 1
test_op 'x=5, x = x+1' # should be 6

# fancy variable name
test_op 'six=6, seven=7, six*seven' # should be 42
test_op '小熊=6, 維尼=7, 小熊*維尼' # should be 42
test_op 'τ=1.618, 3*τ' # should be 3 * 1.618 = 4.854
test_op '$(τ, 1.618), 3*τ()' # shold be 3 * 1.618 = 4.854

sudo rmmod calc
