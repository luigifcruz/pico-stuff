// Initial TinyUSB RNDIS written by Peter Lawrence.
// Modifications were made by Luigi Cruz.

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "bsp/board.h"
#include "tusb.h"

#include "dhserver.h"
#include "dnserver.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"

/* lwip context */
static struct netif netif_data;

/* shared between tud_network_recv_cb() and service_traffic() */
static struct pbuf *received_frame;

/* this is used by this code, ./class/net/net_driver.c, and usb_descriptors.c */
/* ideally speaking, this should be generated from the hardware's unique ID (if available) */
/* it is suggested that the first byte is 0x02 to indicate a link-local address */
const uint8_t tud_network_mac_address[6] = {0x02,0x02,0x84,0x6A,0x96,0x00};

/* network parameters of this MCU */
static const ip_addr_t ipaddr  = IPADDR4_INIT_BYTES(192, 168, 7, 1);
static const ip_addr_t netmask = IPADDR4_INIT_BYTES(255, 255, 255, 0);
static const ip_addr_t gateway = IPADDR4_INIT_BYTES(0, 0, 0, 0);

/* database IP addresses that can be offered to the host; this must be in RAM to store assigned MAC addresses */
static dhcp_entry_t entries[] = {
    /* mac ip address                          lease time */
    { {0}, IPADDR4_INIT_BYTES(192, 168, 7, 2), 24 * 60 * 60 },
    { {0}, IPADDR4_INIT_BYTES(192, 168, 7, 3), 24 * 60 * 60 },
    { {0}, IPADDR4_INIT_BYTES(192, 168, 7, 4), 24 * 60 * 60 },
};

static const dhcp_config_t dhcp_config = {
    .router = IPADDR4_INIT_BYTES(0, 0, 0, 0),  /* router address (if any) */
    .port = 67,                                /* listen port */
    .dns = IPADDR4_INIT_BYTES(192, 168, 7, 1), /* dns server (if any) */
    "usb",                                     /* dns suffix */
    TU_ARRAY_SIZE(entries),                    /* num entry */
    entries                                    /* entries */
};
static err_t linkoutput_fn(struct netif *netif, struct pbuf *p) {
    (void)netif;

    for (;;) {
        /* if TinyUSB isn't ready, we must signal back to lwip that there is nothing we can do */
        if (!tud_ready()) return ERR_USE;

        /* if the network driver can accept another packet, we make it happen */
        if (tud_network_can_xmit()) {
            tud_network_xmit(p, 0 /* unused for this example */);
            return ERR_OK;
        }

        /* transfer execution to TinyUSB in the hopes that it will finish transmitting the prior packet */
        tud_task();
    }
}

static err_t output_fn(struct netif *netif, struct pbuf *p, const ip_addr_t *addr) {
    return etharp_output(netif, p, addr);
}

static err_t netif_init_cb(struct netif *netif) {
    LWIP_ASSERT("netif != NULL", (netif != NULL));
    netif->mtu = CFG_TUD_NET_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;
    netif->state = NULL;
    netif->name[0] = 'E';
    netif->name[1] = 'X';
    netif->linkoutput = linkoutput_fn;
    netif->output = output_fn;
    return ERR_OK;
}

static void init_lwip(void) {
    struct netif *netif = &netif_data;

    lwip_init();

    /* the lwip virtual MAC address must be different from the host's; to ensure this, we toggle the LSbit */
    netif->hwaddr_len = sizeof(tud_network_mac_address);
    memcpy(netif->hwaddr, tud_network_mac_address, sizeof(tud_network_mac_address));
    netif->hwaddr[5] ^= 0x01;

    netif = netif_add(netif, &ipaddr, &netmask, &gateway, NULL, netif_init_cb, ip_input);
    netif_set_default(netif);
}

/* handle any DNS requests from dns-server */
bool dns_query_proc(const char *name, ip_addr_t *addr)
{
  if (0 == strcmp(name, "tiny.usb"))
  {
    *addr = ipaddr;
    return true;
  }
  return false;
}

bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
    /* this shouldn't happen, but if we get another packet before
    parsing the previous, we must signal our inability to accept it */
    if (received_frame) return false;

    if (size) {
        struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);

        if (p) {
            /* pbuf_alloc() has already initialized struct; all we need to do is copy the data */
            memcpy(p->payload, src, size);

            /* store away the pointer for service_traffic() to later handle */
            received_frame = p;
        }
    }

    return true;
}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
    struct pbuf *p = (struct pbuf *)ref;
    struct pbuf *q;
    uint16_t len = 0;

    (void)arg; /* unused for this example */

    /* traverse the "pbuf chain"; see ./lwip/src/core/pbuf.c for more info */
    for(q = p; q != NULL; q = q->next) {
        memcpy(dst, (char *)q->payload, q->len);
        dst += q->len;
        len += q->len;
        if (q->len == q->tot_len) break;
    }

    return len;
}

static void service_traffic(void)
{
    /* handle any packet received by tud_network_recv_cb() */
    if (received_frame) {
        ethernet_input(received_frame, &netif_data);
        pbuf_free(received_frame);
        received_frame = NULL;
        tud_network_recv_renew();
    }

    sys_check_timeouts();
}

void tud_network_init_cb(void) {
    /* if the network is re-initializing and we have a leftover packet, we must do a cleanup */
    if (received_frame) {
        pbuf_free(received_frame);
        received_frame = NULL;
    }
}

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
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    streaming = true;
}

static void stop_stream() {
    adc_run(false);
    adc_fifo_drain();
    dma_channel_set_write_addr(dma_chan_a, &capture_buf_a, false);
    dma_channel_set_write_addr(dma_chan_b, &capture_buf_b, false);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
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
    if (err != ERR_OK && p != NULL)
        goto exception;

    tcp_recved(pcb, p->tot_len);
    tcp_sent(pcb, NULL);

    // The connection is closed if the client sends "X".
    if (((char*)p->payload)[0] == 'X')
        srv_close(pcb);

exception:
    pbuf_free(p);
    return err;
}

static err_t srv_accept(void * arg, struct tcp_pcb * pcb, err_t err) {
    if (err != ERR_OK)
        return err;

    tcp_setprio(pcb, TCP_PRIO_MAX);
    tcp_recv(pcb, srv_receive);
    tcp_err(pcb, srv_err);
    tcp_poll(pcb, NULL, 4);

    client = pcb;
    start_stream();

    return err;
}

bool led_timer(struct repeating_timer *t) {
    if (!streaming) {
        int current = gpio_get(PICO_DEFAULT_LED_PIN);
        gpio_put(PICO_DEFAULT_LED_PIN, !current);
    }
    return true;
}

int main(void) {
    board_init();

    // Init built-in LED.
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Init ADC DMA chain.
    init_adc_dma_chain();

    // Init network stack.
    tusb_init();
    init_lwip();

    // Startup lwIP stack.
    while (!netif_is_up(&netif_data));
    while (dhserv_init(&dhcp_config) != ERR_OK);
    while (dnserv_init(&ipaddr, 53, dns_query_proc) != ERR_OK);

    // Start TCP server.
    struct tcp_pcb* pcb = tcp_new();
    pcb->so_options |= SOF_KEEPALIVE;
    pcb->keep_intvl = 75000000;
    tcp_bind(pcb, IP_ADDR_ANY, 7777);

    // Start listening for connections.
    struct tcp_pcb* listen = tcp_listen(pcb);
    tcp_accept(listen, srv_accept);

    // Start blinking LED indicator.
    add_repeating_timer_ms(250, led_timer, NULL, &timer);

    // Listen to events.
    while (1) {
        tud_task();
        service_traffic();
    }

    return 0;
}