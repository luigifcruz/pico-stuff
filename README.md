# pico-stuff
I add my Pi Pico (RP2040) stuff here.

## Header-Only Libraries
- BMP180

## Installation
```bash
$ git clone --recursive git@github.com:luigifcruz/pico-stuff.git
$ cd pico-stuff
$ mkdir build
$ cd build
$ PICO_SDK_PATH=../pico-sdk cmake ..
$ make -j$(nproc -n)
```

## Debug
For debug add `#define DEBUG` before the `#include` of a header-only library.