#!/bin/bash

NUM_TESTS=1
SIZES=(5000 10000 50000 100000 200000 517402)
#export NUM_QUERIES=1

#declare -a QUERIES=("enron && time && inform && work && call && discuss && meet && week && receiv && dai"
#                    "!(enron && time)" "!enron || !time" "!(enron || time)"
#                    )

# initial compile
(cd ../src/Test && make clean && make)
(cd ../src/Server && make clean && make)

for DATASET_SIZE in "${SIZES[@]}"
do
    export DATASET_DIR="../Data/parsed/$DATASET_SIZE/"

    echo "############################## DATASET SIZE $DATASET_SIZE ##############################"

    for (( i=0; i < $NUM_TESTS; i++ ))
    do
        echo "## Gen test $i ##"
        (cd ../src/Test && ./main)
        echo ""
    done

    echo "############################################"
    echo "############## DONE GEN TESTS ##############"
    echo "############################################"

    for (( i=0; i < $NUM_TESTS; i++ ))
    do
        echo "## SGX test $i ##"
        kill -9 `pidof Server` ; ../src/Server/Server &
        (cd ../src/tsgx/ && make clean > /dev/null && make > /dev/null && (cd build && ./test_bisen))
        echo ""
    done
    echo ""
done
