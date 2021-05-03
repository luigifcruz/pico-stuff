#include <stdio.h>
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "usb_network.h"

uint data_ovf;
uint data_dma;
bool data_val;
bool streaming;
struct repeating_timer timer;

#define CAPTURE_CHANNEL 0
#define CAPTURE_DEPTH 1472

uint dma_chan_a, dma_chan_b;
struct pbuf *pbuf_a, *pbuf_b;

static void dma_handler(uint id) {
    if (data_val) {
        data_ovf += 1;
    }
    data_dma = id;
    data_val = true;
}

static void dma_handler_a() {
    dma_handler(0);
    dma_channel_set_write_addr(dma_chan_a, pbuf_a->payload, false);
    dma_hw->ints0 = 1u << dma_chan_a;
}

static void dma_handler_b() {
    dma_handler(1);
    dma_channel_set_write_addr(dma_chan_b, pbuf_b->payload, false);
    dma_hw->ints1 = 1u << dma_chan_b;
}

static void init_adc_dma_chain() {
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
        pbuf_a->payload,// dst
        &adc_hw->fifo,  // src
        CAPTURE_DEPTH,  // transfer count
        true            // start now
    );

    dma_channel_configure(dma_chan_b, &dma_cfg_b,
        pbuf_b->payload, // dst
        &adc_hw->fifo,  // src
        CAPTURE_DEPTH,  // transfer count
        false           // start now
    );

    dma_channel_set_irq0_enabled(dma_chan_a, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler_a);
    irq_set_priority(DMA_IRQ_0, 0xFF);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_set_irq1_enabled(dma_chan_b, true);
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler_b);
    irq_set_priority(DMA_IRQ_1, 0xFF);
    irq_set_enabled(DMA_IRQ_1, true);

    adc_run(false);
}

static void start_stream(struct tcp_pcb *pcb) {
    if (streaming) {
        return;
    }

    data_ovf = 0;
    data_dma = 0;
    data_val = false;
    streaming = true;

    dma_channel_set_write_addr(dma_chan_a, pbuf_a->payload, true);
    dma_channel_set_write_addr(dma_chan_b, pbuf_b->payload, false);
    adc_run(true);
}

static void stop_stream(struct tcp_pcb *pcb) {
    if (!streaming) {
        return;
    }

    adc_run(false);
    adc_fifo_drain();
    dma_channel_set_write_addr(dma_chan_a, pbuf_a->payload, false);
    dma_channel_set_write_addr(dma_chan_b, pbuf_b->payload, false);

    data_ovf = 0;
    data_dma = 0;
    data_val = false;
    streaming = false;
}

static bool led_timer(struct repeating_timer *t) {
    int status = 1;
    if (streaming) {
        status = !gpio_get(PICO_DEFAULT_LED_PIN);
    }
    gpio_put(PICO_DEFAULT_LED_PIN, status);
    return true;
}

int main(void) {
    // Init built-in LED.
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Init network stack.
    network_init();

    // Allocate zero-copy memory for DMA and UDP.
    pbuf_a = pbuf_alloc(PBUF_RAW, CAPTURE_DEPTH, PBUF_RAM);
    pbuf_b = pbuf_alloc(PBUF_RAW, CAPTURE_DEPTH, PBUF_RAM);

    if (pbuf_a == NULL && pbuf_b == NULL) {
        return 1;
    }

    // Init ADC DMA chain.
    init_adc_dma_chain();

    // Start data UDP port.
    ip_addr_t client;
    struct udp_pcb* dpcb = udp_new();
    IP4_ADDR(&client, 192, 168, 7, 2);
    udp_connect(dpcb, &client, 7778);

    // Start LED indicator.
    add_repeating_timer_ms(250, led_timer, NULL, &timer);

    // Start stream right away.
    start_stream(NULL);

    // Listen to events.
    while (1) {
        if (data_val && streaming) {
            if (data_dma == 0) {
                udp_send(dpcb, pbuf_a);
            }
            if (data_dma == 1) {
                udp_send(dpcb, pbuf_b);
            }

            data_val = false;
        }

        network_step();
    }

    return 0;
}
