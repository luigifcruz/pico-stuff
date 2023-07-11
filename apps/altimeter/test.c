#include <stdio.h>
#include <stdlib.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include <lfs_rp2040.h>
#include <bmp390.h>

static lfs_t lfs;
static struct lfs_config cfg;

uint32_t read_boot_count(bool increment) {
    uint32_t boot_count = 0;
    lfs_file_t boot_count_file;
    lfs_file_open(&lfs, &boot_count_file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &boot_count_file, &boot_count, sizeof(boot_count));

    if (increment) {
        boot_count += 1;
        lfs_file_rewind(&lfs, &boot_count_file);
        lfs_file_write(&lfs, &boot_count_file, &boot_count, sizeof(boot_count));
    }

    lfs_file_close(&lfs, &boot_count_file);

    return boot_count;
}

void reset_recordings() {
    lfs_format(&lfs, &cfg);
    lfs_mount(&lfs, &cfg);
}

void start_altimeter_mode() {
    uint32_t boot_count = read_boot_count(true);
    printf("Current recoding index: %d\n", boot_count);

    char filename[64] = {0};
    sprintf(filename, "REC_%03d", boot_count);
    printf("Creating recording file: %s\n", filename);
    lfs_file_t recording_file;
    lfs_file_open(&lfs, &recording_file, filename, LFS_O_RDWR | LFS_O_CREAT);

    bmp_t bmp;
    bmp.oss = 5;
    bmp.i2c.addr = 0x77;
    bmp.i2c.inst = i2c1;
    bmp.i2c.rate = 400000;
    bmp.i2c.scl = 3;
    bmp.i2c.sda = 2;

    printf("Starting BMP390...\n");
    if (!bmp_init(&bmp)) {
        return;
    }

    while(true) {
        if (!bmp_get_pressure_temperature(&bmp)) {
            continue;
        }

        printf("---------------------------------------------\n");
        printf("Temperature (ºC): %f\n", bmp.temperature);
        printf("Pressure (hPa): %f\n", bmp.pressure);
        printf("Altitude (m): %f\n", bmp.altitude);

        lfs_file_write(&lfs, &recording_file, &bmp.temperature, sizeof(float));
        lfs_file_write(&lfs, &recording_file, &bmp.pressure, sizeof(float));
        lfs_file_sync(&lfs, &recording_file);

        sleep_ms(250);
    }

    lfs_file_close(&lfs, &recording_file);
}

void dump_recording_file(uint32_t index) {
    printf("<======");

    char filename[64] = {0};
    sprintf(filename, "REC_%03d", index);

    lfs_file_t recording_file;
    lfs_file_open(&lfs, &recording_file, filename, LFS_O_RDWR);

    int32_t file_size = lfs_file_size(&lfs, &recording_file);

    uint8_t read_buffer[128];
    for (int32_t seek = 0; seek < file_size; seek += sizeof(read_buffer)) {
        lfs_file_read(&lfs, &recording_file, read_buffer, sizeof(read_buffer));
        
        for (int32_t i = 0; i < sizeof(read_buffer); i++) {
            printf("%02X", read_buffer[i]);
        }
    }

    lfs_file_close(&lfs, &recording_file);

    printf("======>\n");
}

int main(void) {
    stdio_init_all();

    sleep_ms(2500);

    printf("Hello from Pi Pico!\n");

    printf("Checking hardware capabilities...\n");
    lfs_rp2040_init(&cfg);

    printf("Mounting filesystem...\n");
    if (lfs_mount(&lfs, &cfg)) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    printf("Filesystem Usage (blocks): %d/%d\n", lfs_fs_size(&lfs), cfg.block_count);
    printf("Automatically starting altimeter in 5 seconds...\n");
    printf("Press 'X' to enter in CLI mode.\n");

    int elapsed = 0;
    int iteration_time = 50;
    while (getchar_timeout_us(0) != 'X') {
        sleep_ms(iteration_time);
        elapsed += iteration_time;

        if (elapsed >= 5000) {
            start_altimeter_mode();
        }
    }

    printf("Available recordings: %d\n", read_boot_count(false));
    printf("Commands:\n");
    printf("    D - Dump recording file.\n");
    printf("    + - Reset recording counter.\n");

    uint32_t index;

    while(true) {
        char cmd = getchar_timeout_us(0);

        switch(cmd) {
            case 'D':
                printf("Type the recording index.\n");
                scanf("%d", &index);

                if (index > read_boot_count(false)) {
                    printf("Index out of range.\n");
                    break;
                }

                printf("Dumping recording #%d\n", index);
                dump_recording_file(index);
                break;

            case '+':
                printf("Reseting recordings...\n");
                reset_recordings();
                printf("Done! Available recordings: %d\n", read_boot_count(false));
                break;

            default: 
                sleep_ms(30);
                break;
        }
    }

    lfs_unmount(&lfs);

    return 0;
}
