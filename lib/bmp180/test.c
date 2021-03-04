#include <stdio.h>

#define DEBUG
#include "bmp180.h"

int main() {
    stdio_init_all();

    getchar();
    printf("Hello from Pi Pico!\n");

    bmp_t bmp;
    bmp.oss = 0;
    bmp.i2c.addr = 0x77;
    bmp.i2c.inst = i2c1;
    bmp.i2c.rate = 400000;
    bmp.i2c.scl = 27;
    bmp.i2c.sda = 26;

    if (!bmp_init(&bmp))
        return 1;

    while (true) {
        if (!bmp_get_pressure_temperature(&bmp))
            return 1;
        printf("BMP180 Temperature (C): %f\n", bmp.temperature);
        printf("BMP180 Pressure (hPa): %f\n", (float)bmp.pressure / 100.0);
        sleep_ms(250);
    }

    printf("Bye from pico!\n\n");

    return 0;
}
