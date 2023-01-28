#include "lfs_rp2040.h"

// Read a region in a block. Negative error codes are propagated
// to the user.
int lfs_rp2040_read(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, void *buffer, lfs_size_t size) {
    return 0;
}

// Program a region in a block. The block must have previously
// been erased. Negative error codes are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int lfs_rp2040_prog(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, const void *buffer, lfs_size_t size) {
    return 0;
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int lfs_rp2040_erase(const struct lfs_config *c, lfs_block_t block) {
    return 0;
}

// Sync the state of the underlying block device. Negative error codes
// are propagated to the user.
int lfs_rp2040_sync(const struct lfs_config *c) {
    return 0;
}