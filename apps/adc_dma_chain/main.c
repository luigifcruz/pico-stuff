#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#define CAPTURE_CHANNEL 4
#define CAPTURE_DEPTH 10000

uint dma_chan_a, dma_chan_b;
uint8_t capture_buf_a[CAPTURE_DEPTH];
uint8_t capture_buf_b[CAPTURE_DEPTH];

void dma_handler(uint8_t* buffer, int id) {
    printf("DMA IRQ %d [%d %d %d]\n", id, buffer[0], buffer[1], buffer[2]);
}

void dma_handler_a() {
    dma_handler((uint8_t*)&capture_buf_a, 0);
    dma_hw->ints0 = 1u << dma_chan_a;
    dma_channel_set_write_addr(dma_chan_a, &capture_buf_a, false);
}

void dma_handler_b() {
    dma_handler((uint8_t*)&capture_buf_b, 1);
    dma_hw->ints1 = 1u << dma_chan_b;
    dma_channel_set_write_addr(dma_chan_b, &capture_buf_b, false);
}

int main() {
    stdio_init_all();

    getchar();
    printf("Hello from Pi Pico!\n");

    adc_gpio_init(26 + CAPTURE_CHANNEL);
    adc_init();
    adc_select_input(CAPTURE_CHANNEL);
    adc_fifo_setup(
        true,   // Write to FIFO
        true,   // Enable DREQ
        1,      // Trigger DREQ with at least one sample
        false,  // No ERR bit
        true    // Shift each sample by 8 bits
    );
    adc_set_clkdiv(0);

    printf("Arming DMA.\n");
    dma_channel_config dma_cfg_a, dma_cfg_b;

    dma_chan_a = dma_claim_unused_channel(true);
    dma_chan_b = dma_claim_unused_channel(true);

    dma_cfg_a = dma_channel_get_default_config(dma_chan_a);
    dma_cfg_b = dma_channel_get_default_config(dma_chan_b);

    channel_config_set_transfer_data_size(&dma_cfg_a, DMA_SIZE_8);
    channel_config_set_transfer_data_size(&dma_cfg_b, DMA_SIZE_8);

    channel_config_set_read_increment(&dma_cfg_a, false);
    channel_config_set_read_increment(&dma_cfg_b, false);

    channel_config_set_write_increment(&dma_cfg_a, true);
    channel_config_set_write_increment(&dma_cfg_b, true);

    channel_config_set_dreq(&dma_cfg_a, DREQ_ADC);
    channel_config_set_dreq(&dma_cfg_b, DREQ_ADC);

    channel_config_set_chain_to(&dma_cfg_a, dma_chan_b);
    channel_config_set_chain_to(&dma_cfg_b, dma_chan_a);

    dma_channel_configure(dma_chan_a, &dma_cfg_a,
        capture_buf_a,  // dst
        &adc_hw->fifo,  // src
        CAPTURE_DEPTH,  // transfer count
        true            // start now
    );

    dma_channel_configure(dma_chan_b, &dma_cfg_b,
        capture_buf_b,  // dst
        &adc_hw->fifo,  // src
        CAPTURE_DEPTH,  // transfer count
        false           // start now
    );

    dma_channel_set_irq0_enabled(dma_chan_a, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler_a);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_set_irq1_enabled(dma_chan_b, true);
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler_b);
    irq_set_enabled(DMA_IRQ_1, true);

    printf("Start capture.\n");
    adc_run(true);

    while (true)
        tight_loop_contents();

    printf("Bye from pico!\n\n");

    return 0;
}
