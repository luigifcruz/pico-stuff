# Pico Stuff
I add my Pi Pico (RP2040) stuff here. There are complete [apps](/apps) and [libraries](/lib) for sensors or complicated tasks.

## Libraries
- [BMP180](/lib/bmp1880): Header-only library for the BMP180 atmospheric pressure sensor.

## Apps
- [PiccoloSDR](/apps/piccolosdr): A primitive direct-sampling SDR.
- [ADC DMA Chain](/apps/adc_dma_chain): Chained DMA data acquisition from the ADC.
- [Barometer](/apps/barometer): Barometer polling the temperature and atmospheric pressure from a BMP180.
- [Iperf Server](/apps/iperf_server): A tool to measure the performance of the TinyUSB's TCP/IP stack over USB.
- [TCP Server](/apps/tcp_server): A TCP server example to send high-frequency data to the host computer.

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

### Donation
Donations are welcome. I have no commercial interest in this code. Your donation will help me buy more hardware.
- [Patreon](https://www.patreon.com/luigifcruz)
- [Buy Me a Coffee](https://www.buymeacoffee.com/luigi)
- [PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=TAA65AJMC7498&source=url)

### Support
Feel free to hit me up on [Twitter](https://twitter.com/luigifcruz) or [Email](mailto:luigifcruz@gmail.com) if your question isn't answered by this documentation. If you found a bug, please, report it directly on [GitHub Issues](https://github.com/luigifreitas/pisdr-image/issues).

### License
This project is distributed by an [GPL-2.0 License](/LICENSE).

### Disclaimer
This project isn't in any way associated with the Raspberry Pi Foundation.

### Contributing
Everyone is very welcome to contribute to our project.