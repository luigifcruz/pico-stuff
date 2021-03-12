# PiccoloSDR (WIP)
This example is a Raspberry Pico RP2040 working as a basic direct-sampling SDR. The data is sent via USB using the RNDIS protocol to emulate a TCP/IP interface. The ADC speed is limited to 500 ksps. This was tested on Linux but should work fine on Windows. The data can be used with software like the GNU Radio, an example is available [here](/apps/piccolosdr/piccolosdr.grc). It requires an OOT module that can be found [here](https://github.com/ghostop14/gr-grnet).

### Specifications
- 500 ksps Sample-rate
- 250 kHz Bandwidth

### Dependencies Device
- Patched `pico-sdr` and `pico-extras`.
- [USB Network Stack](/lib/networking) Library.

### Usage
This data stream will start when a TCP connection is established. After plugging the device in the USB port of your computer you will be able to open the GNU Radio flowgraph and see the data.

![GNU Radio Example With PiccoloSDR](/apps/piccolosdr/media/gnuradio_example.jpg)