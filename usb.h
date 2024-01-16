#ifndef USB_H
#define USB_H

#include <stdint.h>
#include <avr/pgmspace.h>

#define EORSTE_SET (0b1 << 3)
#define SOFE_SET (0b1 << 2)
#define ADDEN_SET (0b1 << 7);

#define EP0_FIFO_RESET do { UERST |= _BV(EPRST0); } while (false)
#define EP1_FIFO_RESET do { UERST |= _BV(EPRST1); } while (false)
#define EP_FIFO_RESET_COMPLETE do { UERST = 0x00; } while (false)
#define EP_STALL_REQUEST do { UECONX |= _BV(STALLRQ); } while (false)
#define EP_ENABLE do { UECONX |= _BV(EPEN); } while (false)
#define EPEN_SET (0b1 << 0)
#define EPTYPE_CONTROL (0b00 << 6)
#define EPTYPE_INTERRUPT (0b11 << 6)
#define EPDIR_IN (0b1 << 0)
#define EPSIZE_8BYTE (0b000 << 4)
#define EPSIZE_32BYTE (0b010 << 4)
#define EPSIZE_64BYTE (0b011 << 4)
#define EPBK_ONEBANK (0b00 << 2)
#define ALLOC_SET (0b01 << 1)
#define EP_SETUP_ACK do { UEINTX &= ~_BV(RXSTPI); } while (false)
#define EP_OUT_ACK do { UEINTX &= ~_BV(RXOUTI); } while (false)
#define EP_IN_ACK do { UEINTX &= ~_BV(TXINI); } while (false)
#define RXSTPE_SET (0b1 << 3)

typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    union {
        uint16_t wValue;
        struct {
            uint8_t wValueL;
            uint8_t wValueH;
        };
    };
    union {
        uint16_t wIndex;
        struct {
            uint8_t wIndexL;
            uint8_t wIndexH;
        };
    };
    union {
        uint16_t wLength;
        struct {
            uint8_t wLengthL;
            uint8_t wLengthH;
        };
    };
} request_t;

typedef struct [[gnu::packed]] {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} device_descriptor_t;
static_assert(sizeof(device_descriptor_t) == 18);

typedef struct [[gnu::packed]] {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} configuration_descriptor_t;
static_assert(sizeof(configuration_descriptor_t) == 9);

typedef struct [[gnu::packed]] {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} interface_descriptor_t;
static_assert(sizeof(interface_descriptor_t) == 9);

typedef struct [[gnu::packed]] {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} endpoint_descriptor_t;
static_assert(sizeof(endpoint_descriptor_t) == 7);

typedef struct [[gnu::packed]] {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bReportDescriptorType;
    uint16_t wReportDescriptorLength;
} HID_descriptor_t;
static_assert(sizeof(HID_descriptor_t) == 9);

enum {
    DIR_HOST_TO_DEVICE = 0 << 7,
    DIR_DEVICE_TO_HOST = 1 << 7,
    TYPE_STANDARD = 0 << 5,
    TYPE_CLASS = 1 << 5,
    TYPE_VENDOR = 2 << 5,
    RECIPIENT_DEVICE = 0 << 0,
    RECIPIENT_INTERFACE = 1 << 0,
    RECIPIENT_ENDPOINT = 2 << 0,
    RECIPIENT_OTHER = 3 << 0,
};

#define REQ(CODE, DIR, TYPE, RECIPIENT) \
    ((CODE) << 8 | (DIR_##DIR | TYPE_##TYPE | RECIPIENT_##RECIPIENT))

enum {
    GET_STATUS = 0,
    CLEAR_FEATURE = 1,
    SET_FEATURE = 3,
    SET_ADDRESS = 5,
    GET_DESCRIPTOR = 6,
    SET_DESCRIPTOR = 7,
    GET_CONFIGURATION = 8,
    SET_CONFIGURATION = 9,
    GET_INTERFACE = 10,
    SET_INTERFACE = 11,
    SYNCH_FRAME = 12,
    GET_REPORT = 0x01,
    GET_IDLE = 0x02,
    GET_PROTOCOL = 0x03,
    SET_REPORT = 0x09,
    SET_IDLE = 0x0A,
    SET_PROTOCOL = 0x0B,
};

enum {
    DEVICE = 1,
    CONFIGURATION = 2,
    STRING = 3,
    INTERFACE = 4,
    ENDPOINT = 5,
    DEVICE_QUALIFIER = 6,
    OTHER_SPEED_CONFIGURATION = 7,
    INTERFACE_POWER = 8,
    HID = 0x21,
    REPORT = 0x22,
    PHYSICAL_DESCRIPTOR = 0x23,
};

#endif
