#!/bin/bash
cat resetcommand.bin | nc -w 1 127.0.0.1 1234
