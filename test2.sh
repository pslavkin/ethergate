while [ 1 ];
do
   sleep 0.2
   cat samsung.c | nc 192.168.2.10 49154
done

