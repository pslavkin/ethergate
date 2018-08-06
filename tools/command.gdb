break  main
target remote | openocd  -f tiva.cfg -c "gdb_port pipe; log_output openocd.log"
monitor reset halt
set disassemble-next-line on
set logging on
quit

