# ld_preload_spy

This is a quick demo showing the overriding of libc methods
using LD_PRELOAD.

## Building

```shell
make clean
make spy.so
```

## Usage

```shell
LD_PRELOAD=./spy.so curl -s -S -k -X GET http://www.google.com
```