#include <stdio.h>

#define DEBUG
#include "bmp390.h"

int main() {
    stdio_init_all();
    sleep_ms(5000);
    printf("Hello from Pi Pico!\n");

    while(!stdio_usb_connected())
    {
        sleep_ms(100);
    }
    printf("stdio_usb_connected()\n");

    char str[50];
    printf("Enter a string: ");
    gets(str);
    printf("You entered: %s", str);

    bmp_t bmp;
    bmp.oss = 5;
    bmp.i2c.addr = 0x77;
    bmp.i2c.inst = i2c1;
    bmp.i2c.rate = 400000;
    bmp.i2c.scl = 3;
    bmp.i2c.sda = 2;

    if (!bmp_init(&bmp))
        return 1;

    while (true) {
        if (!bmp_get_pressure_temperature(&bmp))
            return 1;
        printf("---------------------------------------------\n");
        printf("Temperature (ºC): %f\n", bmp.temperature);
        printf("Pressure (hPa): %f\n", bmp.pressure);
        printf("Altitude (m): %f\n", bmp.altitude);
    }

    printf("Bye from pico!\n\n");

    return 0;
}
