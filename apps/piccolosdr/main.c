#include <stdio.h>
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "usb_network.h"

bool streaming = false;
struct tcp_pcb* client;
struct repeating_timer timer;

//#define DEBUG
#define CAPTURE_CHANNEL 0
#define CAPTURE_DEPTH 500

uint dma_chan_a, dma_chan_b;
uint8_t capture_buf_a[CAPTURE_DEPTH];
uint8_t capture_buf_b[CAPTURE_DEPTH];

void dma_handler(uint8_t* buffer, int id) {
#ifdef DEBUG
    char str[64];
    int len = sprintf(str, "DMA IRQ %d [%d %d ... %d]\n", id, buffer[0], buffer[1], buffer[CAPTURE_DEPTH-1]);
    tcp_write(client, str, len, 0);
    tcp_output(client);
#else
    tcp_write(client, buffer, CAPTURE_DEPTH, 0x01);
    tcp_output(client);
#endif
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

    adc_run(false);
}

static void start_stream() {
    dma_channel_set_write_addr(dma_chan_a, &capture_buf_a, true);
    dma_channel_set_write_addr(dma_chan_b, &capture_buf_b, false);
    adc_run(true);
    streaming = true;
}

static void stop_stream() {
    adc_run(false);
    adc_fifo_drain();
    dma_channel_set_write_addr(dma_chan_a, &capture_buf_a, false);
    dma_channel_set_write_addr(dma_chan_b, &capture_buf_b, false);
    streaming = false;
}

static void srv_close(struct tcp_pcb *pcb){
    stop_stream();

    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_close(pcb);
}

static void srv_err(void *arg, err_t err) {
    // Probably an indication that the client connection went kaput! Stopping stream...
    srv_close(client);
}

static err_t srv_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (err != ERR_OK && p != NULL) {
        goto exception;
    }

    tcp_recved(pcb, p->tot_len);
    tcp_sent(pcb, NULL);

    // The connection is closed if the client sends "X".
    if (((char*)p->payload)[0] == 'X') {
        srv_close(pcb);
    }

exception:
    pbuf_free(p);
    return err;
}

static err_t srv_accept(void * arg, struct tcp_pcb * pcb, err_t err) {
    if (err != ERR_OK) {
        return err;
    }
        
    tcp_setprio(pcb, TCP_PRIO_MAX);
    tcp_recv(pcb, srv_receive);
    tcp_err(pcb, srv_err);
    tcp_poll(pcb, NULL, 4);

    client = pcb;
    start_stream();

    return err;
}

bool led_timer(struct repeating_timer *t) {
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

    // Init ADC DMA chain.
    init_adc_dma_chain();

    // Init network stack.
    network_init();

    // Start TCP server.
    struct tcp_pcb* pcb = tcp_new();
    pcb->so_options |= SOF_KEEPALIVE;
    pcb->keep_intvl = 75000000;
    tcp_bind(pcb, IP_ADDR_ANY, 7777);

    // Start listening for connections.
    struct tcp_pcb* listen = tcp_listen(pcb);
    tcp_accept(listen, srv_accept);

    // Start LED indicator.
    add_repeating_timer_ms(250, led_timer, NULL, &timer);

    // Listen to events.
    while (1) {
        network_step();
    }

    return 0;
}