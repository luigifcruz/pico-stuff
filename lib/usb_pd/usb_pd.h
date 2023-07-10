#ifndef USB_PD_H
#define USB_PD_H

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

typedef struct {
    uint16_t number_of_data_objects;
    uint16_t last_msg_kind;
    uint16_t last_msg_type;
} usbpd_t;

#define ASSERT_OK(X) { if (X == false) return false; };

#define USBPD_MSG_KIND_DATA    0b10
#define USBPD_MSG_KIND_CONTROL 0b01

#define USBPD_CONTROL_MSG_GOODCRC         0b0001
#define USBPD_CONTROL_MSG_GOTOMIN         0b0010
#define USBPD_CONTROL_MSG_ACCEPT          0b0011
#define USBPD_CONTROL_MSG_REJECT          0b0100
#define USBPD_CONTROL_MSG_PING            0b0101
#define USBPD_CONTROL_MSG_PS_RDY          0b0110
#define USBPD_CONTROL_MSG_GET_SOURCE_CAP  0b0111
#define USBPD_CONTROL_MSG_GET_SINK_CAP    0b1000
#define USBPD_CONTROL_MSG_DR_SWAP         0b1001
#define USBPD_CONTROL_MSG_PR_SWAP         0b1010
#define USBPD_CONTROL_MSG_VCONN_SWAP      0b1011
#define USBPD_CONTROL_MSG_WAIT            0b1100
#define USBPD_CONTROL_MSG_SOFT_RESET      0b1101
#define USBPD_CONTROL_MSG_NOT_SUPPORTED   0b0000

const char* get_control_message_type_name(int message_type) {
    switch (message_type) {
        case USBPD_CONTROL_MSG_GOODCRC:         return "GoodCRC";
        case USBPD_CONTROL_MSG_GOTOMIN:         return "GotoMin";
        case USBPD_CONTROL_MSG_ACCEPT:          return "Accept";
        case USBPD_CONTROL_MSG_REJECT:          return "Reject";
        case USBPD_CONTROL_MSG_PING:            return "Ping";
        case USBPD_CONTROL_MSG_PS_RDY:          return "PS_RDY";
        case USBPD_CONTROL_MSG_GET_SOURCE_CAP:  return "Get_Source_Cap";
        case USBPD_CONTROL_MSG_GET_SINK_CAP:    return "Get_Sink_Cap";
        case USBPD_CONTROL_MSG_DR_SWAP:         return "DR_Swap";
        case USBPD_CONTROL_MSG_PR_SWAP:         return "PR_Swap";
        case USBPD_CONTROL_MSG_VCONN_SWAP:      return "VCONN_Swap";
        case USBPD_CONTROL_MSG_WAIT:            return "Wait";
        case USBPD_CONTROL_MSG_SOFT_RESET:      return "Soft_Reset";
        case USBPD_CONTROL_MSG_NOT_SUPPORTED:   return "Not Supported";
        default:                                return "Unknown";
    }
}

#define USBPD_DATA_MSG_SOURCE_CAPABILITIES  0b0001
#define USBPD_DATA_MSG_REQUEST              0b0010
#define USBPD_DATA_MSG_BIST                 0b0011
#define USBPD_DATA_MSG_SINK_CAPABILITIES    0b0100
#define USBPD_DATA_MSG_BATTERY_STATUS       0b0101
#define USBPD_DATA_MSG_ALERT                0b0110
#define USBPD_DATA_MSG_GET_COUNTRY_INFO     0b0111
#define USBPD_DATA_MSG_COUNTRY_INFO         0b1000
#define USBPD_DATA_MSG_VENDOR_DEFINED       0b1111

const char* get_data_message_type_name(int message_type) {
    switch (message_type) {
        case USBPD_DATA_MSG_SOURCE_CAPABILITIES:  return "Source_Capabilities";
        case USBPD_DATA_MSG_REQUEST:              return "Request";
        case USBPD_DATA_MSG_BIST:                 return "BIST";
        case USBPD_DATA_MSG_SINK_CAPABILITIES:    return "Sink_Capabilities";
        case USBPD_DATA_MSG_BATTERY_STATUS:       return "Battery_Status";
        case USBPD_DATA_MSG_ALERT:                return "Alert";
        case USBPD_DATA_MSG_GET_COUNTRY_INFO:     return "Get_Country_Info";
        case USBPD_DATA_MSG_COUNTRY_INFO:         return "Country_Info";
        case USBPD_DATA_MSG_VENDOR_DEFINED:       return "Vendor_Defined";
        default:                                  return "Unknown";
    }
}

#define USBPD_SPEC_REVISION_1 0b00
#define USBPD_SPEC_REVISION_2 0b01
#define USBPD_SPEC_REVISION_3 0b10

const char* get_specification_revision_name(int revision) {
    switch (revision) {
        case USBPD_SPEC_REVISION_1: return "Revision_1";
        case USBPD_SPEC_REVISION_2: return "Revision_2";
        case USBPD_SPEC_REVISION_3: return "Revision_3";
        default:                    return "Unknown";
    }
}

#define USBPD_DATA_PORT_ROLE_UPSTREAM   0b0
#define USBPD_DATA_PORT_ROLE_DOWNSTREAM 0b1

const char* get_data_port_role_name(int role) {
    switch (role) {
        case USBPD_DATA_PORT_ROLE_UPSTREAM:   return "Upstream";
        case USBPD_DATA_PORT_ROLE_DOWNSTREAM: return "Downstream";
        default:                              return "Unknown";
    }
}

#define USBPD_POWER_PORT_ROLE_SINK   0b0
#define USBPD_POWER_PORT_ROLE_SOURCE 0b1

const char* get_power_port_role_name(int role) {
    switch (role) {
        case USBPD_POWER_PORT_ROLE_SINK:   return "Sink";
        case USBPD_POWER_PORT_ROLE_SOURCE: return "Source";
        default:                           return "Unknown";
    }
}

void split_to_bytes(uint64_t num, uint8_t n_bytes, uint8_t* byte_array) {
    for (int i = 0; i < n_bytes; i++) {
        byte_array[i] = num & 0xFF;
        num >>= 8;
    }
}

bool usb_pd_parse_header(usbpd_t* usbpd, uint8_t* header_bytes) {
    uint16_t header = (header_bytes[1] << 8) | header_bytes[0];

    uint16_t extended = (header >> 15) & 0b1;
    uint16_t num_data_objects = (header >> 12) & 0b1111;
    uint16_t message_id = (header >> 9) & 0b111;
    uint16_t power_role = (header >> 8) & 0b1;
    uint16_t specification_revision = (header >> 6) & 0b11;
    uint16_t data_role = (header >> 5) & 0b1;
    uint16_t message_type = header & 0b1111;

#ifdef DEBUG
    const char* message_type_name;
    const char* specification_revision_name = get_specification_revision_name(specification_revision);
    const char* data_port_role_name = get_data_port_role_name(data_role);
    const char* power_port_role_name = get_power_port_role_name(power_role);

    if (num_data_objects == 0) {
        usbpd->last_msg_kind = USBPD_MSG_KIND_CONTROL;
        message_type_name = get_control_message_type_name(message_type);
    } else {
        usbpd->last_msg_kind = USBPD_MSG_KIND_DATA;
        message_type_name = get_data_message_type_name(message_type);
    }

    printf("=== USB-PD Message Header ================\n");
    printf("Message Kind: %s\n", num_data_objects == 0 ? "Control" : "Data");
    printf("Message Type: %s (%d)\n", message_type_name, message_type);
    printf("Message ID: %d\n", message_id);
    printf("Number of Data Objects: %d\n", num_data_objects);
    printf("Specification Revision: %s (%d)\n", specification_revision_name, specification_revision);
    printf("Power Port Role: %s\n", power_port_role_name);
    printf("Data Port Role: %s\n", data_port_role_name);
    printf("Extended: %s\n", extended ? "YES" : "NO");
    printf("==========================================\n");
#endif

    usbpd->number_of_data_objects = num_data_objects;
    usbpd->last_msg_type = message_type;

    return true;
}

bool usb_pd_parse_pdo_fixed_supply(usbpd_t* usbpd, uint32_t pdo) {
    uint32_t max_current = pdo & 0b1111111111;
    uint32_t voltage = (pdo >> 10) & 0b1111111111;
    uint32_t peak_current = (pdo >> 20) & 0b11;
    uint32_t epr_capable = (pdo >> 23) & 0b1;
    uint32_t uem_supported = (pdo >> 24) & 0b1;
    uint32_t dual_role_power = (pdo >> 25) & 0b1;
    uint32_t usb_comms_capable = (pdo >> 26) & 0b1;
    uint32_t unconstrained_power = (pdo >> 27) & 0b1;
    uint32_t usb_suspend_supported = (pdo >> 28) & 0b1;

    max_current = max_current * 10;
    voltage = voltage * 50;

#ifdef DEBUG
    printf("=== USB-PD Fixed Supply PDO ==============\n");
    printf("Max Current: %.3f A\n", max_current / 1000.0);
    printf("Voltage: %.3f V\n", voltage / 1000.0);
    printf("Peak Current: %s\n", (char*[]){"Not Supported", "Level 1", "Level 2", "Level 3"}[peak_current]);
    if (voltage == 5000) {
        printf("==========================================\n");
        printf("EPR Capable: %s\n", epr_capable ? "YES" : "NO");
        printf("Unchunked Extended Messages Supported: %s\n", uem_supported ? "YES" : "NO");
        printf("Dual-Role Power: %s\n", dual_role_power ? "YES" : "NO");
        printf("USB Communications Capable: %s\n", usb_comms_capable ? "YES" : "NO");
        printf("Uncostrained Power: %s\n", unconstrained_power ? "YES" : "NO");
        printf("USB Suspend Supported: %s\n", usb_suspend_supported ? "YES" : "NO");
    }
    printf("==========================================\n");
#endif

    return true;
}

bool usb_pd_parse_pdo_pps(usbpd_t* usbpd, uint32_t pdo) {
    uint32_t max_voltage = (pdo >> 17) & 0b11111111;
    uint32_t min_voltage = (pdo >> 8) & 0b11111111;
    uint32_t max_current = pdo & 0b1111111;

    max_voltage = max_voltage * 100;
    min_voltage = min_voltage * 100;
    max_current = max_current * 50;

#ifdef DEBUG
    printf("=== USB-PD Programmable Power Supply PDO =\n");
    printf("Voltage: %.3f V to %.3f V\n", min_voltage / 1000.0, max_voltage / 1000.0);
    printf("Max Current: %.3f A\n", max_current / 1000.0);
    printf("==========================================\n");
#endif

    return true;
}

bool usb_pd_parse_pdo_vps(usbpd_t* usbpd, uint32_t pdo) {
    uint32_t max_voltage = (pdo >> 20) & 0b1111111111;
    uint32_t min_voltage = (pdo >> 10) & 0b1111111111;
    uint32_t max_current = pdo & 0b1111111111;

    max_voltage = max_voltage * 50;
    min_voltage = min_voltage * 50;
    max_current = max_current * 10;

#ifdef DEBUG
    printf("=== USB-PD Variable Power Supply PDO =====\n");
    printf("Voltage: %.3f V to %.3f V\n", min_voltage / 1000.0, max_voltage / 1000.0);
    printf("Max Current: %.3f A\n", max_current / 1000.0);
    printf("==========================================\n");
#endif

    return true;
}

bool usb_pd_parse_pdo_bps(usbpd_t* usbpd, uint32_t pdo) {
    uint32_t max_voltage = (pdo >> 20) & 0b1111111111;
    uint32_t min_voltage = (pdo >> 10) & 0b1111111111;
    uint32_t max_current = pdo & 0b1111111111;

    max_voltage = max_voltage * 50;
    min_voltage = min_voltage * 50;
    max_current = max_current * 250;

#ifdef DEBUG
    printf("=== USB-PD Battery Power Supply PDO ======\n");
    printf("Voltage: %.3f V to %.3f V\n", min_voltage / 1000.0, max_voltage / 1000.0);
    printf("Max Current: %.3f W\n", max_current / 1000.0);
    printf("==========================================\n");
#endif

    return true;
}

bool usb_pd_parse_pdo(usbpd_t* usbpd, uint8_t* pdo_bytes) {
    uint32_t pdo = (pdo_bytes[3] << 24) | (pdo_bytes[2] << 16) | (pdo_bytes[1] << 8) | pdo_bytes[0];

    if (usbpd->last_msg_type == USBPD_DATA_MSG_SOURCE_CAPABILITIES) {
        uint8_t pdo_type = (pdo >> 30) & 0b11;

        if (pdo_type == 0b00) {
            return usb_pd_parse_pdo_fixed_supply(usbpd, pdo);
        }
        
        if (pdo_type == 0b01) {
            return usb_pd_parse_pdo_bps(usbpd, pdo);
        }

        if (pdo_type == 0b10) {
            return usb_pd_parse_pdo_vps(usbpd, pdo);
        }

        if (pdo_type == 0b11) {
            return usb_pd_parse_pdo_pps(usbpd, pdo);
        }
    }

    if (usbpd->last_msg_type == USBPD_DATA_MSG_VENDOR_DEFINED) {
        printf("[USB-PD] Vendor PDO are not supported yet.\n");
    }

    return false;
}

bool usb_pd_generate_header(usbpd_t* usbpd,
                            uint8_t* payload,
                            uint16_t number_of_objects, 
                            uint16_t message_id,
                            uint16_t power_role,
                            uint16_t specification_name,
                            uint16_t data_role,
                            uint16_t data_message_type) {
#ifdef DEBUG
    printf("[USB-PD] Generating header.\n");
#endif
    uint16_t header = 0x0000;

    header |= (0b0 << 15);
    header |= (number_of_objects << 12);
    header |= (message_id << 9);
    header |= (power_role << 8);
    header |= (specification_name << 6);
    header |= (data_role << 5);
    header |= (data_message_type);

    split_to_bytes(header, 2, payload);
    usb_pd_parse_header(usbpd, payload);

    return true;
}

bool usb_pd_parse_rdo(usbpd_t* usbpd, uint8_t* rdo_bytes) {
    uint32_t pdo = (rdo_bytes[3] << 24) | (rdo_bytes[2] << 16) | (rdo_bytes[1] << 8) | rdo_bytes[0];

    return true;
}

bool usb_pd_parse_rdo_fvs(usbpd_t* usbpd, uint32_t header) {
    uint16_t object_position = (header >> 28) & 0b1111;
    bool give_back = (header >> 27) & 0b1;
    bool capability_mismatch = (header >> 26) & 0b1;
    bool usb_communications_capable = (header >> 25) & 0b1;
    bool no_usb_suspend = (header >> 24) & 0b1;
    bool unchunked_messages_support = (header >> 23) & 0b1;
    bool epr_support = (header >> 22) & 0b1;
    uint16_t output_current = ((header >> 10) & 0b1111111111) * 10;
    uint16_t max_output_current = (header & 0b1111111111) * 10;

#ifdef DEBUG
    printf("=== USB-PD Var/Fix Power Supply RDO ======\n");
    printf("Object Position: %d\n", object_position);
    printf("Give Back: %s\n", give_back ? "YES" : "NO");
    printf("Capability Mismatch: %s\n", capability_mismatch ? "YES" : "NO");
    printf("USB Communications Capable: %s\n", usb_communications_capable ? "YES" : "NO");
    printf("No USB Suspend: %s\n", no_usb_suspend ? "YES" : "NO");
    printf("Unchunked Messages Support: %s\n", unchunked_messages_support ? "YES" : "NO");
    printf("EPR Support: %s\n", epr_support ? "YES" : "NO");
    printf("Output Current: %.3f A\n", output_current / 1000.0);
    printf("Max Output Current: %.3f A\n", max_output_current / 1000.0);
    printf("==========================================\n");
#endif

    return true;
}

bool usb_pd_parse_rdo_pps(usbpd_t* usbpd, uint32_t header) {
    uint16_t object_position = (header >> 28) & 0b1111;
    bool capability_mismatch = (header >> 26) & 0b1;
    bool usb_communications_capable = (header >> 25) & 0b1;
    bool no_usb_suspend = (header >> 24) & 0b1;
    bool unchunked_messages_support = (header >> 23) & 0b1;
    bool epr_support = (header >> 22) & 0b1;
    uint16_t output_voltage = ((header >> 9) & 0b111111111111) * 20;
    uint16_t output_current = (header & 0b1111111) * 50;

#ifdef DEBUG
    printf("=== USB-PD Programmable Power Supply RDO =\n");
    printf("Object Position: %d\n", object_position);
    printf("Capability Mismatch: %s\n", capability_mismatch ? "YES" : "NO");
    printf("USB Communications Capable: %s\n", usb_communications_capable ? "YES" : "NO");
    printf("No USB Suspend: %s\n", no_usb_suspend ? "YES" : "NO");
    printf("Unchunked Messages Support: %s\n", unchunked_messages_support ? "YES" : "NO");
    printf("EPR Support: %s\n", epr_support ? "YES" : "NO");
    printf("Output Voltage: %.3f V\n", output_voltage / 1000.0);
    printf("Output Current: %.3f A\n", output_current / 1000.0);
    printf("==========================================\n");
#endif

    return true;
}

bool usb_pd_generate_rdo_pps(usbpd_t* usbpd, 
                             uint8_t* payload,
                             uint16_t object_position,
                             bool capability_mismatch,
                             bool usb_communications_capable,
                             bool no_usb_suspend,
                             bool unchunked_messages_support,
                             bool epr_support,
                             uint16_t output_voltage,
                             uint16_t output_current) {
#ifdef DEBUG
    printf("[USB-PD] Generating PPS RDO.\n");
#endif
    uint32_t header = 0x00000000;

    header |= (object_position << 28);
    header |= (capability_mismatch << 26);
    header |= (usb_communications_capable << 25);
    header |= (no_usb_suspend << 24);
    header |= (unchunked_messages_support << 23);
    header |= (epr_support << 22);
    header |= ((output_voltage / 20) << 9);
    header |= ((output_current / 50) << 0);

    split_to_bytes(header, 4, payload);
    usb_pd_parse_rdo_pps(usbpd, header);

    return true;
}

bool usb_pd_generate_rdo_fvs(usbpd_t* usbpd, 
                             uint8_t* payload, 
                             uint16_t object_position,
                             bool give_back,
                             bool capability_mismatch,
                             bool usb_communications_capable,
                             bool no_usb_suspend,
                             bool unchunked_messages_support,
                             bool epr_support,
                             uint16_t output_current,
                             uint16_t max_output_current) {
#ifdef DEBUG
    printf("[USB-PD] Generating FVS RDO.\n");
#endif
    uint32_t header = 0x00000000;

    header |= (object_position << 28);
    header |= (give_back << 27);
    header |= (capability_mismatch << 26);
    header |= (usb_communications_capable << 25);
    header |= (no_usb_suspend << 24);
    header |= (unchunked_messages_support << 23);
    header |= (epr_support << 22);
    header |= ((output_current / 10) << 10);
    header |= ((max_output_current / 10) << 0);

    split_to_bytes(header, 4, payload);
    usb_pd_parse_rdo_fvs(usbpd, header);

    return true;
}

#endif
