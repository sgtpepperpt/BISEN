#!/bin/bash

export DATASET_SIZE=50

declare -a QUERIES=("enron && time" "enron && time && inform && work && call" "enron && time && inform && work && call && discuss && meet && week && receiv && dai"
                    "enron || time" "enron || time || call || work || inform" "enron || time || inform || work || call || discuss || meet || week || receiv || dai"
                    "(call || enron) && (time || attach)" "(call || enron) && (time || attach) && (inform || work) && (meet || week)"
                    "(call && enron) || (time && attach)" "(call && enron) || (time && attach) || (inform && work) || (meet && week)"
                    "!enron && !time" "!(enron && time)" "!enron || !time" "!(enron || time)"
                    )

for QRY in "${QUERIES[@]}"
do
    echo "############################## QUERY $QRY ##############################"
    export QUERY=$QRY
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
