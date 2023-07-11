#ifndef FUSB_H
#define FUSB_H

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define FUSB_PACKET_START_MARKER 0xe0

typedef struct {
    int addr;
    int rate;
    int scl;
    int sda;
    i2c_inst_t* inst;
} i2c_t;

typedef struct {
    i2c_t i2c;
    bool source;
    uint8_t revision;
    bool auto_crc;
    uint8_t polarity;
    uint16_t packets;
} fusb_t;

static fusb_t* ff;

#define ASSERT_OK(X) { if (X == false) return false; };

void read_i2c(fusb_t* fusb, uint32_t size, uint8_t addr, void* data) {
    i2c_write_blocking(fusb->i2c.inst, fusb->i2c.addr, &addr, 1, true);
    i2c_read_blocking(fusb->i2c.inst, fusb->i2c.addr, data, size, false);
}

void write_reg_i2c(fusb_t* fusb, uint8_t addr, uint8_t reg) {
    uint8_t payload[] = { addr, reg };
    i2c_write_blocking(fusb->i2c.inst, fusb->i2c.addr, payload, 2, false);
}

void write_i2c(fusb_t* fusb, uint32_t size, void* data) {
    i2c_write_blocking(fusb->i2c.inst, fusb->i2c.addr, data, size, false);
}

bool fusb_check_chip_id(fusb_t* fusb) {
    uint8_t chip_id;
    read_i2c(fusb, 1, 0x01, &chip_id);
#ifdef DEBUG
    printf("[FUSB] Chip ID: 0x%x\n", chip_id);
#endif
    return true; 
}

bool fusb_reset(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Device reset.\n");
#endif
    uint8_t reg;
    read_i2c(fusb, 1, 0x0c, &reg);
    reg |= 0x01;
    write_reg_i2c(fusb, 0x0c, reg);
    return true;
}

bool fusb_pd_reset(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Power delivery reset.\n");
#endif
    uint8_t reg;
    read_i2c(fusb, 1, 0x0c, &reg);
    reg |= 0x02;
    write_reg_i2c(fusb, 0x0c, reg);
    return true;
}

bool fusb_power_on(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Power device on.\n");
#endif
    uint8_t reg;
    read_i2c(fusb, 1, 0x0b, &reg);
    reg |= 0x0f;
    write_reg_i2c(fusb, 0x0b, reg);
    return true;
}

bool fusb_config(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Configuring device.\n");
#endif
    write_reg_i2c(fusb, 0x0a, 0b01101111);
    write_reg_i2c(fusb, 0x0e, 0b10111110);
    write_reg_i2c(fusb, 0x0f, 0b00000001);
    write_reg_i2c(fusb, 0x06, 0b00000100);
    write_reg_i2c(fusb, 0x08, 0b00000100);
    write_reg_i2c(fusb, 0x09, 0b00000111);
    return true;
}

bool fusb_pd_config(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Configuring PD.\n");
#endif
    uint8_t cmd = 0x00;

    if (fusb->source) {
        cmd |= 0b10010000;
    }

    cmd |= fusb->revision << 5;

    if (fusb->auto_crc) {
        cmd |= 0b00000100;
    }

    cmd |= fusb->polarity;

    write_reg_i2c(fusb, 0x03, cmd);

    return true;
}

bool fusb_flush_rx_fifo(fusb_t* fusb) {
    uint8_t reg;
    read_i2c(fusb, 1, 0x07, &reg);
    reg |= 0x04;
    write_reg_i2c(fusb, 0x07, reg);
    return true;
}

bool fusb_flush_tx_fifo(fusb_t* fusb) {
    uint8_t reg;
    read_i2c(fusb, 1, 0x06, &reg);
    reg |= 0x40;
    write_reg_i2c(fusb, 0x06, reg);
    return true;
}

bool fusb_connect_cc(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Connecting CC.\n");
#endif
    uint8_t reg;
    read_i2c(fusb, 1, 0x02, &reg);
    reg &= (~0b1100 & 0xFF);
    reg |= (fusb->polarity << 2);
    write_reg_i2c(fusb, 0x02, reg);
    return true;
}

bool fusb_pd_enable_toggle(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Toggle enabled.\n");
#endif
    uint8_t reg;
    read_i2c(fusb, 1, 0x08, &reg);
    reg |= 0x01;
    write_reg_i2c(fusb, 0x08, reg);
    return true;
}

bool fusb_pd_disable_toggle(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Toggle disabled.\n");
#endif
    uint8_t reg;
    read_i2c(fusb, 1, 0x08, &reg);
    reg &= ~0x01;
    write_reg_i2c(fusb, 0x08, reg);
    return true;
}

bool fusb_pd_handle_toggle_snk(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Handle toggle sink.\n");
#endif
    // Set polarity and pull.
    ASSERT_OK(fusb_connect_cc(fusb));
    ASSERT_OK(fusb_pd_config(fusb));
    
    // Check if CC is still present.
    uint8_t reg;
    read_i2c(fusb, 1, 0x40, &reg);
    if (!(reg & 0x03)) {
        printf("Restarting toggling.\n");
        fusb_pd_enable_toggle(fusb);
        return true;
    }
    
    // Disable toggle.
    fusb_pd_disable_toggle(ff);

    return true;
}

void i2c_isr(unsigned int gpio, uint32_t events) {
    uint8_t reg[7];
    read_i2c(ff, 7, 0x3c, &reg);

#define TRACE_DEBUG

#ifdef TRACE_DEBUG
    printf("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
        reg[0], reg[1], reg[2], reg[3], reg[4], reg[5], reg[6]);

    printf("I_HARDRESET %x\n", (reg[2] & 0x01) >> 0);
    printf("HARDRESET %x\n", (reg[0] & 0x01) >> 0);
    
    printf("I_TOGDONE %x\n", (reg[2] & 0x40) >> 6);
    printf("TOGSS %x\n", (reg[1] & 0x38) >> 3);

    printf("I_COMP_CHNG %x\n", (reg[6] & 0x20) >> 5);
    printf("COMP %x\n", (reg[4] & 0x20) >> 5);
#endif

    if (reg[2] & 0x40) {
        switch ((reg[1] & 0x38) >> 3) {
            case 0b101:
#ifdef DEBUG
                printf("[FUSB] Sink on CC1 attached.\n");
#endif
                ff->polarity = 0x01;
                fusb_pd_handle_toggle_snk(ff);
                break;
            case 0b110:
#ifdef DEBUG
                printf("[FUSB] Sink on CC2 attached.\n");
#endif
                ff->polarity = 0x02;
                fusb_pd_handle_toggle_snk(ff);
                break;
            default:
                printf("[FUSB] Not defined toggle state detected.\n");
                break;
        }
    }

    if ((reg[2] & 0x01) && (reg[0] & 0x01)) {
#ifdef DEBUG
        printf("[FUSB] Hard reset.\n");
#endif
        if (!ff->packets) {
            fusb_pd_reset(ff);
        }
    }

    if (reg[6] & 0x10) {
#ifdef DEBUG
        printf("[FUSB] New packet (%d).\n", ff->packets);
#endif
        ff->packets += 1;
    }

    if ((reg[6] & 0x80) && !(reg[4] & 0x80)) {
#ifdef DEBUG
        printf("[FUSB] Detach occured.\n");
#endif
        ff->polarity = 0x00;
        fusb_connect_cc(ff);
        fusb_pd_config(ff);
        fusb_flush_tx_fifo(ff);
        fusb_flush_rx_fifo(ff);
        fusb_pd_reset(ff);
        fusb_pd_enable_toggle(ff);
    }
}

bool fusb_init(fusb_t* fusb) {
#ifdef DEBUG
    printf("[FUSB] Connecting device.\n");
#endif
    // Initializing the I2C.
    i2c_init(fusb->i2c.inst, fusb->i2c.rate);

    // Configuring the I2C pins.
    gpio_set_function(fusb->i2c.scl, GPIO_FUNC_I2C);
    gpio_set_function(fusb->i2c.sda, GPIO_FUNC_I2C);
    gpio_pull_up(fusb->i2c.scl);
    gpio_pull_up(fusb->i2c.sda);

    // Setting up the interrupt handler.
    gpio_set_irq_enabled_with_callback(20, GPIO_IRQ_EDGE_FALL, true, &i2c_isr);
    ff = fusb;

#ifdef DEBUG
    printf("[FUSB] Initializing device.\n");
#endif

    fusb->auto_crc = true;
    fusb->revision = 0x10;
    fusb->source = false;
    fusb->packets = 0;

    ASSERT_OK(fusb_check_chip_id(fusb));
    ASSERT_OK(fusb_reset(fusb));
    ASSERT_OK(fusb_check_chip_id(fusb));

    ASSERT_OK(fusb_config(fusb));
    ASSERT_OK(fusb_power_on(fusb));
    ASSERT_OK(fusb_flush_tx_fifo(fusb));
    ASSERT_OK(fusb_flush_rx_fifo(fusb));

    ASSERT_OK(fusb_pd_enable_toggle(fusb));
    
    return true;
}

bool fusb_read_fifo(fusb_t* fusb, uint32_t size, void* data) {
    read_i2c(fusb, size, 0x43, data);
    return true;
}

bool fusb_write_fifo(fusb_t* fusb, uint32_t size, void* data) {
#ifdef DEBUG
    printf("[FUSB] Writing FIFO (size = %d).\n", size);
#endif
    uint8_t sop[] = { 0x43, 0x12, 0x12, 0x12, 0x13, 0x80 | size };
    write_i2c(fusb, 6, sop);

    uint8_t payload[70] = { 0x43 };
    memcpy(payload + 1, data, size);
    write_i2c(fusb, size + 1, payload);

    uint8_t eop[] = { 0x43, 0xff, 0x14, 0xfe, 0xa1 };
    write_i2c(fusb, 5, eop);

    return true;
}

#endif
