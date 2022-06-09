#!/bin/bash
for i in {1..48}
do
  for j in {1..50}#Transmit test data 100 packets
  do
    cat testframe_012.bin | nc -w 1 127.0.0.1 1234
    cat testframe_FED.bin | nc -w 1 127.0.0.1 1234
  done
  
  #Don't need to do anything in bash for rx. Handled in gnuradio.
  #TODO make interaction with python script

  #Transparent mode timeout
  sleep 30s

  sleep 90m
done