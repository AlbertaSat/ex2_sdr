#!/bin/bash
cat pipe_command_default.bin | nc -w 1 127.0.0.1 1234
for i in {1..100}; do cat testframe.bin | nc -w 1 127.0.0.1 1234; done
