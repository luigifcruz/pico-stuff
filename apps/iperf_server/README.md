# Iperf Server
This is a tool to measure the network performance using `iperf2`. This program will make the Pico work like a network router when the device is plugged into a computer USB port using the RNDIS protocol. This was tested on Linux. This example will also create a sample HTTP page on 192.168.7.1.

### Dependencies
- Patched `pico-sdr` and `pico-extras`.
- [USB Network Stack](/lib/networking) Library.

### Usage
This program will start automatically. No user interaction with the device is needed. The device won't produce an output. You can measure the network speed with `iperf2` using the command below. Expect speeds between 6-10 Mbps. This is a physical limitation of the Full Speed USB present on the RP2040.

```bash
$ iperf -c 192.168.7.1
```