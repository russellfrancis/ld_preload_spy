#!/usr/bin/env bash
LD_PRELOAD=./spy.so curl -s -S -k -X GET http://www.google.com > /dev/null