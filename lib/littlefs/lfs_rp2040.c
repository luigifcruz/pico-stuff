#include "lfs_rp2040.h"

#define DEBUG 1

extern char __flash_binary_end;

static uint storage_get_flash_capacity() {
    uint8_t rxbuf[4];
    uint8_t txbuf[4] = {0x9f};
    flash_do_cmd(txbuf, rxbuf, 4);
    return 1 << rxbuf[3];
}

static struct {
    uintptr_t lfs_start_pos;
    uintptr_t lfs_end_pos;
    uintptr_t lfs_start_addr;
    uintptr_t lfs_end_addr;
    uint lfs_block_size;
    uint lfs_read_size;
    uint lfs_write_size;
} _lfs_rp2040_state;

const struct lfs_config lfs_rp2040_init() {
    const uintptr_t flash_capacity = storage_get_flash_capacity();
    const uintptr_t bin_start = (uintptr_t)XIP_BASE;
    const uintptr_t bin_end = (uintptr_t)&__flash_binary_end;

    _lfs_rp2040_state.lfs_read_size = FLASH_PAGE_SIZE;
    _lfs_rp2040_state.lfs_write_size = FLASH_PAGE_SIZE;

    _lfs_rp2040_state.lfs_block_size = FLASH_SECTOR_SIZE;
    _lfs_rp2040_state.lfs_start_addr = (bin_end - 1u + _lfs_rp2040_state.lfs_block_size) & 
                                           -_lfs_rp2040_state.lfs_block_size ;
    _lfs_rp2040_state.lfs_end_addr = bin_start + flash_capacity;

    _lfs_rp2040_state.lfs_start_pos = _lfs_rp2040_state.lfs_start_addr - bin_start;
    _lfs_rp2040_state.lfs_end_pos = _lfs_rp2040_state.lfs_end_addr - bin_start;
    
    const uint lfs_block_count = (_lfs_rp2040_state.lfs_end_addr - 
                                     _lfs_rp2040_state.lfs_start_addr) / 
                                         _lfs_rp2040_state.lfs_block_size ;

#ifdef DEBUG
    printf("Flash capacity    : 0x%08x\n", flash_capacity);
    printf("Binary start      : 0x%08x\n", bin_start);
    printf("Binary end        : 0x%08x\n", bin_end);
    printf("LFS start address : 0x%08x\n", _lfs_rp2040_state.lfs_start_addr);
    printf("LFS end address   : 0x%08x\n", _lfs_rp2040_state.lfs_end_addr);
    printf("LFS start pos     : 0x%08x\n", _lfs_rp2040_state.lfs_start_pos);
    printf("LFS end pos       : 0x%08x\n", _lfs_rp2040_state.lfs_end_pos);
    printf("LFS block count   : 0x%08x\n", lfs_block_count);
#endif

    const struct lfs_config cfg = {
        .read  = lfs_rp2040_read,
        .prog  = lfs_rp2040_prog,
        .erase = lfs_rp2040_erase,
        .sync  = lfs_rp2040_sync,

        .read_size = _lfs_rp2040_state.lfs_read_size,
        .prog_size = _lfs_rp2040_state.lfs_write_size,
        .block_size = _lfs_rp2040_state.lfs_block_size,
        .block_count = lfs_block_count,
        .cache_size = _lfs_rp2040_state.lfs_write_size,
        .lookahead_size = 16,
        .block_cycles = 500,
    };

    return cfg;
}

int lfs_rp2040_read(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, void *buffer, lfs_size_t size) {
#ifdef DEBUG
    printf("[LFS I/O] - READ - B: 0x%08x | O: 0x%08x | S: 0x%08x\n", block, off, size);
#endif

    const uintptr_t base_addr = _lfs_rp2040_state.lfs_start_addr;
    const uintptr_t block_offset = _lfs_rp2040_state.lfs_block_size * block;

    memcpy(buffer, (void*)(base_addr + block_offset + off), size);

    return LFS_ERR_OK;
}

int lfs_rp2040_prog(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, const void *buffer, lfs_size_t size) {
#ifdef DEBUG
    printf("[LFS I/O] - PROG - B: 0x%08x | O: 0x%08x | S: 0x%08x\n", block, off, size);
#endif

    uint32_t ints = save_and_disable_interrupts();

    const uintptr_t base_pos = _lfs_rp2040_state.lfs_start_pos;
    const uintptr_t block_offset = _lfs_rp2040_state.lfs_block_size * block;

    flash_range_program(base_pos + block_offset + off, buffer, size);

    restore_interrupts(ints);

    return LFS_ERR_OK;
}

int lfs_rp2040_erase(const struct lfs_config *c, lfs_block_t block) {
#ifdef DEBUG
    printf("[LFS I/O] - ERASE - B: 0x%08x\n", block);
#endif

    uint32_t ints = save_and_disable_interrupts();

    const uintptr_t block_size = _lfs_rp2040_state.lfs_block_size;
    const uintptr_t base_pos = _lfs_rp2040_state.lfs_start_pos;
    const uintptr_t block_offset = _lfs_rp2040_state.lfs_block_size * block;

    flash_range_erase(base_pos + block_offset, block_size);

    restore_interrupts(ints);

    return LFS_ERR_OK;
}

int lfs_rp2040_sync(const struct lfs_config *c) {
#ifdef DEBUG
    printf("[LFS I/O] - SYNC\n");
#endif
    return LFS_ERR_OK;
}