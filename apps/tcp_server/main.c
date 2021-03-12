#include <stdio.h>
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "usb_network.h"
#include "lwip/tcp.h"

struct tcp_pcb* client;
struct repeating_timer timer;

bool send_timer(struct repeating_timer *t) {
    const float conversion_factor = 3.3f / (1 << 12);
    float ADC_voltage = adc_read() * conversion_factor;

    char str[64];
    int len = sprintf(str, "TEMP: %f °C\n", 27 - (ADC_voltage - 0.706) / 0.001721);
    tcp_write(client, str, len, 0);
    tcp_output(client);

    return true;
}

static void srv_close(struct tcp_pcb *pcb){
    // Cancel send timer.
    cancel_repeating_timer(&timer);

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

    // Start send timer.
    add_repeating_timer_ms(50, send_timer, NULL, &timer);

    return err;
}

int main(void) {
    // Init network RNDIS stack.
    network_init();

    // Start TCP server.
    struct tcp_pcb* pcb = tcp_new();
    pcb->so_options |= SOF_KEEPALIVE;
    pcb->keep_intvl = 75000000;
    tcp_bind(pcb, IP_ADDR_ANY, 7777);

    // Start listening for connections.
    struct tcp_pcb* listen = tcp_listen(pcb);
    tcp_accept(listen, srv_accept);

    // Start ADC.
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    // Listen to events.
    while (1) {
        network_step();
    }

    return 0;
}