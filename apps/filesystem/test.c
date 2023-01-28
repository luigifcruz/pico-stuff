#include <stdio.h>
#include <stdlib.h>

#include "hardware/flash.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"

#include <lfs.h>
#include <lfs_rp2040.h>

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read  = lfs_rp2040_read,
    .prog  = lfs_rp2040_prog,
    .erase = lfs_rp2040_erase,
    .sync  = lfs_rp2040_sync,

    // block device configuration
    .read_size = FLASH_PAGE_SIZE,
    .prog_size = FLASH_PAGE_SIZE,
    .block_size = FLASH_SECTOR_SIZE,
    .block_count = 128,
    .cache_size = FLASH_PAGE_SIZE,

    .lookahead_size = 16,
    .block_cycles = 500,
};

uint storage_get_flash_capacity() {
    uint8_t rxbuf[4];
    uint8_t txbuf[] = {0x9f};
    flash_do_cmd(txbuf, rxbuf, 4);
    return 1 << rxbuf[3];
}

// entry point
int main(void) {
    stdio_init_all();
    
    while (getchar_timeout_us(0) != 'X') {
        sleep_ms(10);
    }

    printf("%d\n", storage_get_flash_capacity());

    printf("Hello from Pi Pico!\n");

    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    printf("%d\n\n", err);

    return 0;

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
}
