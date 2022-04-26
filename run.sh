#!/usr/bin/env bash
LD_PRELOAD=./spy.so curl -s -S -k --http1.1 -X GET http://www.google.com > /dev/null