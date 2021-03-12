# USB Network Stack
This is a helper library utilizing the TinyUSB RNDIS protocol to create a network interface via the USB port of the Pico. The network speed is between 6-10 Mbps. This is a limitation imposed by the Full Speed USB present on the RP2040. A good example of how to use this helper library is the [tcp_server](/apps/tcp_server) app.

# Dependencies
- Patched `pico-sdr` and `pico-extras`.
- [USB Network Stack](/lib/networking) Library.

# Apps Using This Library
- [PiccoloSDR](/apps/piccolosdr): A primitive direct-sampling SDR.
- [Iperf Server](/apps/iperf_server): A tool to measure the performance of the TinyUSB's TCP/IP stack over USB.
- [TCP Server](/apps/tcp_server): A TCP server example to send high-frequency data to the host computer.

# Usage 
```c
#include "usb_network.h"

int main() {
    network_init();

    while (1) {
        network_step();
    }
}
```