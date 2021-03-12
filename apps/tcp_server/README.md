# TCP Server
This is a TCP Server that will work with the USB Network Stack library to provide a TCP/IP connection between the host (computer) and the device (Pico). This example is a demonstration of how to send high-frequency data to the host using a TCP connection.

### Dependencies
- Patched `pico-sdr` and `pico-extras`.
- [USB Network Stack](/lib/networking) Library.

### Usage
This program will start when a TCP connection is established. For this example, we are going to use Netcat. The command bellow should connect the host with the device. When the connection is opened, the device will start to stream data from the internal temperature sensor with ASCII characters. Binary data can also be sent.

```bash
$ netcat 192.168.7.1 7777
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
TEMP: 23.393365 °C
```