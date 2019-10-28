while [ 1 ];
do
   sleep 0.2
   cat test_string.txt | nc 192.168.2.10 49152
done

