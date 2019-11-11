#!/bin/bash
input="test_file.txt"
while true
do
while IFS= read -r line
do
   sleep 0.01
     echo "$line"
done < "$input"
done
