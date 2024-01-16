#include <avr/pgmspace.h>
#include "usb.h"

static const device_descriptor_t device_descriptor PROGMEM = {
    .bLength = sizeof(device_descriptor_t),
    .bDescriptorType = DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = 64,
    .idVendor = 0xF055,
    .idProduct = 0x0000,
    .bcdDevice = 0x0100,
    .iManufacturer = 0,
    .iProduct = 0,
    .iSerialNumber = 0,
    .bNumConfigurations = 1,
};

static const configuration_descriptor_t configuration_descriptor PROGMEM = {
    .bLength = sizeof(configuration_descriptor_t),
    .bDescriptorType = CONFIGURATION,
    .wTotalLength = sizeof(configuration_descriptor_t) + sizeof(interface_descriptor_t) + sizeof(endpoint_descriptor_t) + sizeof(HID_descriptor_t),
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0b10000000,
    .bMaxPower = 100 / 2,
};

static const interface_descriptor_t interface_descriptor PROGMEM = {
    .bLength = sizeof(interface_descriptor_t),
    .bDescriptorType = INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = 0x03,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
};

static const endpoint_descriptor_t endpoint_descriptor PROGMEM = {
    .bLength = sizeof(endpoint_descriptor_t),
    .bDescriptorType = ENDPOINT,
    .bEndpointAddress = 0b10000001,
    .bmAttributes = 0b00000011,
    .wMaxPacketSize = 0b00000000'00100000,
    .bInterval = 16,
};

static const uint8_t report_descriptor[] PROGMEM = {
    0b0000'01'01, 0x01, /* Usage Page (Generic Desktop) */
    0b0000'10'01, 0x06, /* Usage (Keyboard) */
    0b1010'00'01, 0x01, /* Collection (Application) */
    0b0000'01'01, 0x07, /* Usage Page (Keyboard/Keypad) */
    0b0001'10'01, 0xE0, /*   Usage Minimum (0xE0) */
    0b0010'10'01, 0xE7, /*   Usage Maximum (0xE7) */
    0b0001'01'01,    0, /*   Logical Minimum (0) */
    0b0010'01'01,    1, /*   Logical Maximum (1) */
    0b0111'01'01,    1, /*   Report Size (1) */
    0b1001'01'01,    8, /*   Report Count (8) */
    0b1000'00'01, 0x02, /*   Input (Data, Variable, Absolute) */
    0b0001'10'01, 0x00, /*   Usage Minimum (0x00) */
    0b0010'10'01, 0xA4, /*   Usage Maximum (0xA4) */
    0b0001'01'01,    0, /*   Logical Minimum (0) */
    0b0010'01'01,    1, /*   Logical Maximum (1) */
    0b0111'01'01,    1, /*   Report Size (1) */
    0b1001'01'01,  165, /*   Report Count (165) */
    0b1000'00'01, 0x02, /*   Input (Data, Variable, Absolute) */
    0b0111'01'01,    1, /*   Report Size (1) */
    0b1001'01'01,    3, /*   Report Count (3) */
    0b1000'00'01, 0x03, /*   Input (Constant, Variable, Absolute) */
    0b1100'00'00,       /* End Collection */
};

static const HID_descriptor_t HID_descriptor PROGMEM = {
    .bLength = sizeof(HID_descriptor_t),
    .bDescriptorType = HID,
    .bcdHID = 0x0111,
    .bCountryCode = 0,
    .bNumDescriptors = 1,
    .bReportDescriptorType = REPORT,
    .wReportDescriptorLength = sizeof(report_descriptor),
};

