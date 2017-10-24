#!/bin/bash

SIZES=(5000 10000 20000 50000 100000 200000)

for DATASET_SIZE in "${SIZES[@]}"
do
    echo "############################## DATASET SIZE $DATASET_SIZE ##############################"
    export DATASET_SIZE=$DATASET_SIZE

    for i in {0..5}
    do
        echo "## Gen test $i ##"
        (cd ../src/Test && make && ./main)
        echo ""
    done

    echo "############################################################"

    for i in {0..5}
    do
        echo "## SGX test $i ##"
        kill -9 `pidof Server` ; ../src/Server/Server > /dev/null &
        (cd ../src/tsgx/ && make clean > /dev/null && make > /dev/null && (cd build && ./test_bisen))
        echo ""
    done

    echo ""
done
