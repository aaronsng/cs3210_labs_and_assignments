#!/bin/bash

for i in {112..1..-8}
    do
        ./goi_cuda sample_inputs/sample7.in test7.out 360 1 1 $i 1 1
        while read line; do
            # reading each line
            echo $line
            if [[ $line == *"1263450"* ]]; then
                echo "CORRECT :)"
            fi 
        done < "test7.out"
    done

for i in {360..1..-20}
    do
        ./goi_cuda sample_inputs/sample7.in test7.out $i 1 1 32 1 1
        while read line; do
            # reading each line
            echo $line
            if [[ $line == *"1263450"* ]]; then
                echo "CORRECT :)"
            fi 
        done < "test7.out"
    done
