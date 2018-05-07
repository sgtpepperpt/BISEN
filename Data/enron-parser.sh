#!/bin/bash

for file in $(find $1 -type f); do
    NEW_NAME=$(echo $file | sed 's/\//_/g' | sed 's/\.//g')
    cp ${file} $2/"${NEW_NAME}".txt
done
