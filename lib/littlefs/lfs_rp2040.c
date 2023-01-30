#include "lfs_rp2040.h"

extern char __flash_binary_end;

static uint storage_get_flash_capacity() {
    uint8_t rxbuf[4] = {0};
    uint8_t txbuf[4] = {0x9f};
    flash_do_cmd(txbuf, rxbuf, 4);
    return 1 << rxbuf[3];
}

static struct {
    uint32_t lfs_start_pos;
    uint32_t lfs_end_pos;
    uint32_t lfs_start_addr;
    uint32_t lfs_end_addr;
    uint32_t lfs_block_size;
    uint32_t lfs_read_size;
    uint32_t lfs_write_size;
} _lfs_rp2040_state;

void lfs_rp2040_init(struct lfs_config* cfg) {

    uint32_t ints = save_and_disable_interrupts();
    const uintptr_t flash_capacity = storage_get_flash_capacity();
    restore_interrupts(ints);

    const uint32_t bin_start = (uint32_t)XIP_BASE;
    const uint32_t bin_end = (uint32_t)&__flash_binary_end + 0x40000;

    _lfs_rp2040_state.lfs_read_size = FLASH_PAGE_SIZE;
    _lfs_rp2040_state.lfs_write_size = FLASH_PAGE_SIZE;

    _lfs_rp2040_state.lfs_block_size = FLASH_SECTOR_SIZE;
    _lfs_rp2040_state.lfs_start_addr = (bin_end - 1u + _lfs_rp2040_state.lfs_block_size) & 
                                           -_lfs_rp2040_state.lfs_block_size;
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

    cfg->read  = lfs_rp2040_read;
    cfg->prog  = lfs_rp2040_prog;
    cfg->erase = lfs_rp2040_erase;
    cfg->sync  = lfs_rp2040_sync;

    cfg->read_size = _lfs_rp2040_state.lfs_read_size;
    cfg->prog_size = _lfs_rp2040_state.lfs_write_size;
    cfg->block_size = _lfs_rp2040_state.lfs_block_size;
    cfg->block_count = lfs_block_count;
    cfg->cache_size = _lfs_rp2040_state.lfs_write_size;
    cfg->lookahead_size = 16;
    cfg->block_cycles = 500;
}

int lfs_rp2040_read(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, void *buffer, lfs_size_t size) {
#ifdef DEBUG
    printf("[LFS I/O] - READ - B: 0x%08x | O: 0x%08x | S: 0x%08x\n", block, off, size);
#endif

    const uint32_t base_addr = _lfs_rp2040_state.lfs_start_addr;
    const uint32_t block_offset = _lfs_rp2040_state.lfs_block_size * block;

    uint32_t ints = save_and_disable_interrupts();
    memcpy(buffer, (void*)(base_addr + block_offset + off), size);
    restore_interrupts(ints);

    return LFS_ERR_OK;
}

int lfs_rp2040_prog(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, const void *buffer, lfs_size_t size) {
#ifdef DEBUG
    printf("[LFS I/O] - PROG - B: 0x%08x | O: 0x%08x | S: 0x%08x\n", block, off, size);
#endif  

    const uint32_t base_pos = _lfs_rp2040_state.lfs_start_pos;
    const uint32_t block_offset = _lfs_rp2040_state.lfs_block_size * block;

    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(base_pos + block_offset + off, buffer, size);
    restore_interrupts(ints);

    return LFS_ERR_OK;
}

int lfs_rp2040_erase(const struct lfs_config *c, lfs_block_t block) {
#ifdef DEBUG
    printf("[LFS I/O] - ERASE - B: 0x%08x\n", block);
#endif

    const uint32_t block_size = _lfs_rp2040_state.lfs_block_size;
    const uint32_t base_pos = _lfs_rp2040_state.lfs_start_pos;
    const uint32_t block_offset = _lfs_rp2040_state.lfs_block_size * block;

    uint32_t ints = save_and_disable_interrupts();
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
