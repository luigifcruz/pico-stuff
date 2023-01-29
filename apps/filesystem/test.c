#include <stdio.h>
#include <stdlib.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include <lfs_rp2040.h>

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// entry point
int main(void) {
    stdio_init_all();
    
    while (getchar_timeout_us(0) != 'X') {
        sleep_ms(10);
    }

    printf("Hello from Pi Pico!\n");

    const struct lfs_config cfg = lfs_rp2040_init();

    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    printf("%d\n\n", err);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // release any resources we were using
    lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %d\n", boot_count);

    while(true) {
        sleep_ms(1000);
    }
}
