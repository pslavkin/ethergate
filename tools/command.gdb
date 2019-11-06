break  main
target remote | openocd  -f tiva.cfg -c "gdb_port pipe; log_output openocd.log"
monitor reset halt
set logging on

