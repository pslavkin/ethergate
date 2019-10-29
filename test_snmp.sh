while [ 1 ]; do sleep 0.2; snmpwalk -v 2c -c public 192.168.2.10 iso; done;
