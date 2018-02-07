#!/bin/bash

# makes server print index size
export NUM_QUERIES=1

NUM_TESTS=2
SIZES=(5000 10000 20000 50000 100000 200000 517402)

# initial compile
(cd ../src/Test && make clean && make)
(cd ../src/Server && make clean && make)

for DATASET_SIZE in "${SIZES[@]}"
do
    echo "############################## DATASET SIZE $DATASET_SIZE ##############################"
    export DATASET_DIR="../Data/parsed/$DATASET_SIZE/"

    for (( i=0; i < $NUM_TESTS; i++ ))
    do
        echo "## Gen test $i ##"
        (cd ../src/Test && ./main)
        echo ""
    done

    echo "############################################################"
    echo "###################### DONE GEN TESTS ######################"
    echo "############################################################"

    for (( i=0; i < $NUM_TESTS; i++ ))
    do
        echo "## SGX test $i ##"
        kill -9 `pidof Server` ; ../src/Server/Server &
        (cd ../src/tsgx/ && make clean > /dev/null && make > /dev/null && (cd build && ./test_bisen))
        echo ""
    done

    echo ""
done
