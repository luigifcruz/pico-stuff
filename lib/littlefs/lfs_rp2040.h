#ifndef LFS_RP2040_H
#define LFS_RP2040_H

#include <stdio.h>
#include <stdlib.h>

#include <lfs.h>
#include <lfs_util.h>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"

const struct lfs_config lfs_rp2040_init();

int lfs_rp2040_read(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, void *buffer, lfs_size_t size);

int lfs_rp2040_prog(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, const void *buffer, lfs_size_t size);

int lfs_rp2040_erase(const struct lfs_config *c, lfs_block_t block);

int lfs_rp2040_sync(const struct lfs_config *c);

#endif