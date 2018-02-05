#!/bin/bash

SIZES=(5000 10000 20000 50000 100000 200000 517402)
export NUM_QUERIES=4

declare -a QUERIES=("enron && time" "enron && time && inform && work && call" "enron && time && inform && work && call && discuss && meet && week && receiv && dai"
                    "enron || time" "enron || time || call || work || inform" "enron || time || inform || work || call || discuss || meet || week || receiv || dai"
                    "(call || enron) && (time || attach)" "(call || enron) && (time || attach) && (inform || work) && (meet || week)"
                    "(call && enron) || (time && attach)" "(call && enron) || (time && attach) || (inform && work) || (meet && week)"
                    "!enron && !time" "!(enron && time)" "!enron || !time" "!(enron || time)"
                    )

#declare -a QUERIES=("!enron && !time"
#                    "!enron && !time && !inform && !work && !call"
#                    "!enron && !time && !inform && !work && !call && !discuss && !meet && !week && !receiv && !dai"
#                    )

for DATASET_SIZE in "${SIZES[@]}"
do
    export DATASET_SIZE=$DATASET_SIZE

    echo "############################## DATASET SIZE $DATASET_SIZE ##############################"
    for QRY in "${QUERIES[@]}"
    do
        echo "###################### QUERY $QRY ######################"
        export QUERY=$QRY
        for i in {0..5}
        do
            echo "## Gen test $i ##"
            (cd ../src/Test && make && ./main)
            echo ""
        done

        echo "############################################"
        echo "############## DONE GEN TESTS ##############"
        echo "############################################"

        for i in {0..5}
        do
            echo "## SGX test $i ##"
            kill -9 `pidof Server` ; ../src/Server/Server &
            (cd ../src/tsgx/ && make clean > /dev/null && make > /dev/null && (cd build && ./test_bisen))
            echo ""
        done

        echo "#################### END QUERY $QRY ####################"
    done
    echo "############################ END DATASET SIZE $DATASET_SIZE ############################"
done
