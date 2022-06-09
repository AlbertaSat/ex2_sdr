#!/bin/bash
cat pipe_command_mode5.bin | nc -w 1 127.0.0.1 1234
cat ../../../ex2_ground_station_software/output2.bin | nc -w 1 127.0.0.1 1235
