#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "usb.h"
#include "usb_descriptor.h"
#include "utility.h"

uint8_t usb_configuration_value = 0x00;
extern volatile uint8_t usb_ep_data_buffer[1 + ((164 + 4) / 8)];
extern volatile bool usb_ep_data_ready;

ISR(USB_GEN_vect, ISR_BLOCK) {
    if (bit_is_set(UDINT, EORSTI)) {
        UDINT &= ~_BV(EORSTI);

        UENUM = 0;
        EP_ENABLE;
        UECFG0X = EPTYPE_CONTROL;
        UECFG1X = EPSIZE_64BYTE | EPBK_ONEBANK | ALLOC_SET;
        if (bit_is_clear(UESTA0X, CFGOK)) {
            return;
        }
        EP0_FIFO_RESET;
        EP_FIFO_RESET_COMPLETE;
        UEIENX = RXSTPE_SET;

        UENUM = 1;
        EP_ENABLE;
        UECONX = EPEN_SET;
        UECFG0X = EPTYPE_INTERRUPT | EPDIR_IN;
        UECFG1X = EPSIZE_32BYTE | EPBK_ONEBANK | ALLOC_SET;
        if (bit_is_clear(UESTA0X, CFGOK)) {
            return;
        }
        EP1_FIFO_RESET;
        EP_FIFO_RESET_COMPLETE;

        UDIEN = EORSTE_SET | SOFE_SET;
    }
    if (bit_is_set(UDINT, SOFI)) {
        UDINT &= ~_BV(SOFI);
        if (usb_configuration_value) {
            /* TODO: implement Start Of Frame behavior */
            UENUM = 1;
            if (usb_ep_data_ready && bit_is_set(UEINTX, TXINI)) {
                EP_IN_ACK;
                if (bit_is_set(UEINTX, RWAL)) {
                    for (uint8_t i = 0; i < sizeof(usb_ep_data_buffer); ++i) {
                        UEDATX = usb_ep_data_buffer[i];
                    }
                }
                usb_ep_data_ready = false;
                UEINTX &= ~_BV(FIFOCON);
            }
        }
    }
}

ISR(USB_COM_vect, ISR_BLOCK) {
    UENUM = 0;
    if (bit_is_set(UEINTX, RXSTPI)) {
        request_t req;
        req.bmRequestType = UEDATX;
        req.bRequest = UEDATX;
        req.wValueL = UEDATX;
        req.wValueH = UEDATX;
        req.wIndexL = UEDATX;
        req.wIndexH = UEDATX;
        req.wLengthL = UEDATX;
        req.wLengthH = UEDATX;
        req.wLength = req.wLengthH << 8 | req.wLengthL;

        switch (req.bRequest << 8 | req.bmRequestType) {
            case REQ(CLEAR_FEATURE, HOST_TO_DEVICE, STANDARD, DEVICE):
            case REQ(CLEAR_FEATURE, HOST_TO_DEVICE, STANDARD, INTERFACE):
            case REQ(CLEAR_FEATURE, HOST_TO_DEVICE, STANDARD, ENDPOINT):
                EP_STALL_REQUEST;
                break;
            case REQ(GET_CONFIGURATION, DEVICE_TO_HOST, STANDARD, DEVICE):
                EP_SETUP_ACK;
                loop_until_bit_is_set(UEINTX, TXINI);
                UEDATX = usb_configuration_value;
                EP_IN_ACK;
                loop_until_bit_is_set(UEINTX, RXOUTI);
                EP_OUT_ACK;
                break;
            case REQ(GET_DESCRIPTOR, DEVICE_TO_HOST, STANDARD, DEVICE):
                switch (req.wValueH) {
                    case DEVICE:
                        EP_SETUP_ACK;
                        if (req.wLength > sizeof(device_descriptor)) {
                            req.wLength = sizeof(device_descriptor);
                        }
                        for (uint16_t i = 0; i < req.wLength; ++i) {
                            UEDATX = pgm_read_byte((uint8_t *)&device_descriptor + i);
                        }
                        EP_IN_ACK;
                        loop_until_bit_is_set(UEINTX, RXOUTI);
                        EP_OUT_ACK;
                        break;
                    case CONFIGURATION:
                        EP_SETUP_ACK;
                        if (req.wLength == sizeof(configuration_descriptor)) {
                            for (uint8_t i = 0; i < req.wLength; ++i) {
                                UEDATX = pgm_read_byte((uint8_t *)&configuration_descriptor + i);
                            }
                        } else {
                            for (uint8_t i = 0; i < sizeof(configuration_descriptor_t); ++i) {
                                UEDATX = pgm_read_byte((uint8_t *)&configuration_descriptor + i);
                            }
                            for (uint8_t i = 0; i < sizeof(interface_descriptor_t); ++i) {
                                UEDATX = pgm_read_byte((uint8_t *)&interface_descriptor + i);
                            }
                            for (uint8_t i = 0; i < sizeof(HID_descriptor_t); ++i) {
                                UEDATX = pgm_read_byte((uint8_t *)&HID_descriptor + i);
                            }
                            for (uint8_t i = 0; i < sizeof(endpoint_descriptor_t); ++i) {
                                UEDATX = pgm_read_byte((uint8_t *)&endpoint_descriptor + i);
                            }
                        }
                        EP_IN_ACK;
                        loop_until_bit_is_set(UEINTX, RXOUTI);
                        EP_OUT_ACK;
                        break;
                    case STRING:
                    case INTERFACE:
                    case ENDPOINT:
                    case DEVICE_QUALIFIER:
                    case OTHER_SPEED_CONFIGURATION:
                    case INTERFACE_POWER:
                        EP_STALL_REQUEST;
                        break;
                }
                break;
            case REQ(GET_INTERFACE, DEVICE_TO_HOST, STANDARD, INTERFACE):
                EP_SETUP_ACK;
                loop_until_bit_is_set(UEINTX, TXINI);
                UEDATX = 0x00; /* No alternate setting is supported */
                EP_IN_ACK;
                loop_until_bit_is_set(UEINTX, RXOUTI);
                EP_OUT_ACK;
                break;
            case REQ(GET_STATUS, DEVICE_TO_HOST, STANDARD, DEVICE):
            case REQ(GET_STATUS, DEVICE_TO_HOST, STANDARD, INTERFACE):
            case REQ(GET_STATUS, DEVICE_TO_HOST, STANDARD, ENDPOINT):
                EP_SETUP_ACK;
                loop_until_bit_is_set(UEINTX, TXINI);
                /* always 0x0000 status no matter of recipient */
                UEDATX = 0x00;
                UEDATX = 0x00;
                EP_IN_ACK;
                loop_until_bit_is_set(UEINTX, RXOUTI);
                EP_OUT_ACK;
                break;
            case REQ(SET_ADDRESS, HOST_TO_DEVICE, STANDARD, DEVICE):
                UDADDR = req.wValueL;
                EP_SETUP_ACK;
                loop_until_bit_is_set(UEINTX, TXINI);
                EP_IN_ACK;
                loop_until_bit_is_set(UEINTX, TXINI);
                UDADDR |= ADDEN_SET;
                break;
            case REQ(SET_CONFIGURATION, HOST_TO_DEVICE, STANDARD, DEVICE):
                EP_SETUP_ACK;
                usb_configuration_value = req.wValueL;
                loop_until_bit_is_set(UEINTX, TXINI);
                EP_IN_ACK;
                break;
            case REQ(SET_DESCRIPTOR, HOST_TO_DEVICE, STANDARD, DEVICE):
            case REQ(SET_FEATURE, HOST_TO_DEVICE, STANDARD, DEVICE):
            case REQ(SET_FEATURE, HOST_TO_DEVICE, STANDARD, INTERFACE):
            case REQ(SET_FEATURE, HOST_TO_DEVICE, STANDARD, ENDPOINT):
            case REQ(SET_INTERFACE, HOST_TO_DEVICE, STANDARD, INTERFACE):
            case REQ(SYNCH_FRAME, DEVICE_TO_HOST, STANDARD, ENDPOINT):
                EP_STALL_REQUEST;
                break;
            case REQ(GET_DESCRIPTOR, DEVICE_TO_HOST, STANDARD, INTERFACE):
                switch (req.wValueH) {
                    case HID:
                        EP_STALL_REQUEST;
                        break;
                    case REPORT:
                        EP_SETUP_ACK;
                        if (req.wLength > sizeof(report_descriptor)) {
                            req.wLength = sizeof(report_descriptor);
                        }
                        for (uint8_t i = 0; i < req.wLength; ++i) {
                            UEDATX = pgm_read_byte(report_descriptor + i);
                        }
                        EP_IN_ACK;
                        loop_until_bit_is_set(UEINTX, RXOUTI);
                        EP_OUT_ACK;
                        break;
                    case PHYSICAL_DESCRIPTOR:
                    default:
                        EP_STALL_REQUEST;
                        break;
                }
                break;
            case REQ(SET_DESCRIPTOR, DEVICE_TO_HOST, STANDARD, INTERFACE):
            case REQ(GET_REPORT, DEVICE_TO_HOST, CLASS, INTERFACE):
            case REQ(GET_IDLE, DEVICE_TO_HOST, CLASS, INTERFACE):
            case REQ(GET_PROTOCOL, DEVICE_TO_HOST, CLASS, INTERFACE):
            case REQ(SET_REPORT, HOST_TO_DEVICE, CLASS, INTERFACE):
            case REQ(SET_IDLE, HOST_TO_DEVICE, CLASS, INTERFACE):
            case REQ(SET_PROTOCOL, HOST_TO_DEVICE, CLASS, INTERFACE):
            default:
                EP_STALL_REQUEST;
                break;
        }
    }

    if (bit_is_set(UEINTX, RXSTPI)) {
        UEINTX &= ~_BV(RXSTPI);
        UECONX |= _BV(STALLRQ);
    }
}
