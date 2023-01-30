# Pico Stuff
I add my Pi Pico (RP2040) stuff here. There are complete [apps](/apps) and [libraries](/lib) for sensors or complicated tasks.

## Libraries
- [BMP180](/lib/bmp180): Header-only library for the BMP180 atmospheric pressure and temperature sensor.
- [BMP390](/lib/bmp390): Header-only library for the BMP390 atmospheric pressure and temperature sensor.
- [USB Network Stack](/lib/usb_network_stack): Library using TinyUSB's implementation of the RNDIS protocol to enable network over USB.
- [LittleFS](/lib/littlefs): A simple non-volatile filesystem based on LittleFS. It uses the internal flash.

## Apps
- [PiccoloSDR](/apps/piccolosdr): A primitive direct-sampling SDR.
- [ADC DMA Chain](/apps/adc_dma_chain): Chained DMA data acquisition from the ADC.
- [Barometer](/apps/barometer): Read temperature and atmospheric pressure from a BMP180.
- [Iperf Server](/apps/iperf_server): A tool to measure the performance of the TinyUSB's TCP/IP stack over USB.
- [TCP Server](/apps/tcp_server): A TCP server example to send high-frequency data to the host computer.
- [Filesystem](/apps/filesystem): A simple non-volatile filesystem based on LittleFS. It uses the internal flash.
- [Altimeter](/apps/altimeter): A simple altimeter for rockets, kites, balloons, etc.

## Installation
Some projects may require a patched version of the `pico-sdk` or `pico-extras`.

```bash
$ git clone --recursive git@github.com:luigifcruz/pico-stuff.git
$ cd pico-stuff
$ mkdir build
$ cd build
$ PICO_SDK_PATH=../pico-sdk cmake ..
$ make -j$(nproc -n)
```

## About the project
This project was created and maintained since 2021 by [Luigi Cruz](https://luigi.ltd).

### Support
Feel free to hit me up on [Twitter](https://twitter.com/luigifcruz) or [Email](mailto:luigifcruz@gmail.com) if your question isn't answered by this documentation. If you found a bug, please, report it directly on [GitHub Issues](https://github.com/luigifreitas/pisdr-image/issues).

### License
This project is distributed by an [GPL-2.0 License](/LICENSE).

### Disclaimer
This project isn't in any way associated with the Raspberry Pi Foundation.

### Contributing
Everyone is very welcome to contribute to our project.
