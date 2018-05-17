#!/bin/bash

INPUT_FILE=$1.in
OUTPUT_FILE=$1.png

gnuplot -e "inputfile='$INPUT_FILE'" -e "outputfile='$OUTPUT_FILE'" -e "cols='$2'" separated.gnu
