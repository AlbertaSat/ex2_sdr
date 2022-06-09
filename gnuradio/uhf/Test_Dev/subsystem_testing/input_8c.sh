#!/bin/bash
cat pipe_command.bin | nc -w 1 127.0.0.1 1234
cat enablebeacon.bin | nc -w 1 127.0.0.1 1234