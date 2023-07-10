#include <stdio.h>
#include <stdlib.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#define DEBUG 1
#include "fusb.h"
#include "usb_pd.h"

int main(void) {
    stdio_init_all();

    sleep_ms(2500);

    printf("Hello from Pi Pico!\n");

    fusb_t fusb;
    fusb.i2c.addr = 0x25;
    fusb.i2c.inst = i2c1;
    fusb.i2c.rate = 1000000;
    fusb.i2c.scl = 18;
    fusb.i2c.sda = 19;

    usbpd_t usbpd;

    printf("Starting FUSB302...\n");
    if (!fusb_init(&fusb)) {
        return 1;
    }

    int i = 0;
    while (1) {
        while (fusb.packets == 0) {
            sleep_ms(15);
        }
        fusb.packets -= 1;

        uint8_t sop[4];
        fusb_read_fifo(&fusb, 1,  &sop);

        if (sop[0] == 0xe0) {
            fusb_read_fifo(&fusb, 2, &sop);
            usb_pd_parse_header(&usbpd, sop);
            for (uint16_t i = 0; i < usbpd.number_of_data_objects; i++) {
                fusb_read_fifo(&fusb, 4, &sop);
                usb_pd_parse_pdo(&usbpd, sop);
            }
            fusb_read_fifo(&fusb, 4, &sop);

            if (usbpd.last_msg_kind == USBPD_MSG_KIND_DATA && 
                usbpd.last_msg_type == USBPD_DATA_MSG_SOURCE_CAPABILITIES) {
                uint8_t payload[6];
                usb_pd_generate_header(&usbpd, 
                                       payload, 
                                       1,
                                       0,
                                       USBPD_POWER_PORT_ROLE_SINK,
                                       USBPD_SPEC_REVISION_3,
                                       USBPD_DATA_PORT_ROLE_UPSTREAM,
                                       USBPD_DATA_MSG_REQUEST);
                usb_pd_generate_rdo_fvs(&usbpd, payload + 2, 2, false, false, false, false, false, false, 1000, 1000);
                // usb_pd_generate_rdo_pps(&usbpd, payload + 2, 6, false, false, false, false, false, 11000, 0);
                fusb_write_fifo(&fusb, 6, payload);
            }
        }

    }

    printf("Goodbye!\n");

    return 0;
}
