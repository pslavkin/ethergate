#!/bin/bash
while IFS='' read -r line || [[ -n "$line" ]]; do
    echo "$line";
    sleep 0.001;
done < $1
