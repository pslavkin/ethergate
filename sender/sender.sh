#!/bin/bash
while IFS='' read -r line || [[ -n "$line" ]]; do
    echo "$line";
    sleep 0.5;
done < $1
sleep 2;
