#!/usr/bin/env bash

# Check the availability of 'bc' utility
command -v bc >/dev/null 2>&1 || \
    { echo >&2 "This script requires 'bc' but it's not installed.  Aborting.";
      exit 1; }

# Transfer the self-defined representation to real number
fromfixed() {
    local ret=$1
    local NAN_INT=31
    local INF_INT=47

    num=$(($ret >> 4))
    frac=$(($ret & 15))
    neg=$((($frac & 8) >> 3))

    [[ $neg -eq 1 ]] && frac=$((-((~$frac & 15) + 1)))

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
