#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include "utility.h"
#include "usb.h"
#include "usb_hid_keys.h"

#define COLUMN_COUNT 14
#define ROW_COUNT 5

uint8_t buffer[COLUMN_COUNT] = { 0, };
uint8_t keymap[COLUMN_COUNT][ROW_COUNT] = {
    { KEY_LEFTALT, KEY_GRAVE, KEY_TAB, KEY_BACKSPACE, KEY_LEFTSHIFT },
    { KEY_F1, KEY_1, KEY_Q, KEY_A, KEY_Z },
    { KEY_F2, KEY_2, KEY_W, KEY_S, KEY_X },
    { KEY_F3, KEY_3, KEY_E, KEY_D, KEY_C },
    { KEY_F4, KEY_4, KEY_R, KEY_F, KEY_V },
    { KEY_F5, KEY_5, KEY_T, KEY_G, KEY_B },
    { KEY_F6, KEY_6, KEY_SPACE, KEY_APOSTROPHE, KEY_LEFTCTRL },
    { KEY_F7, KEY_7, KEY_RIGHTBRACE, KEY_LEFTBRACE, KEY_RIGHTCTRL },
    { KEY_F8, KEY_8, KEY_Y, KEY_H, KEY_N },
    { KEY_F9, KEY_9, KEY_U, KEY_J, KEY_M },
    { KEY_F10, KEY_0, KEY_I, KEY_K, KEY_COMMA },
    { KEY_F11, KEY_MINUS, KEY_O, KEY_L, KEY_DOT },
    { KEY_F12, KEY_EQUAL, KEY_P, KEY_SEMICOLON, KEY_SLASH },
    { KEY_RIGHTMETA, KEY_ESC, KEY_BACKSLASH, KEY_ENTER, KEY_RIGHTSHIFT },
};

volatile bool usb_ep_data_ready = false;
static_assert((164 + 4) % 8 == 0);
volatile uint8_t usb_ep_data_buffer[1 + ((164 + 4) / 8)] = { 0, };

void usb_init(void) {
    UHWCON |= _BV(UVREGE); /* Power-On USB pads regulator */
    PLLCSR |= _BV(PINDIV); /* PLL Input Prescaler 1:2 (16Mhz clock source) */
    PLLCSR |= _BV(PLLE); /* Enable PLL */
    loop_until_bit_is_set(PLLCSR, PLOCK); /* Check PLL lock */
    USBCON |= _BV(OTGPADE); /* Enable USB VBUS Pad */
    USBCON |= _BV(USBE); /* Enable USB interface(?) */
    USBCON &= ~_BV(FRZCLK); /* unfreeze USB clock */
    UDCON &= ~_BV(LSM); /* full-speed */
    UDCON &= ~_BV(DETACH); /* Attach USB device */
    UDIEN |= _BV(EORSTE) | _BV(SOFE); /* Enable End of Reset, Start of Frame interrupts */
}

void matrix_init(void) {
    /*
    ** Key Matrix Configuration
    **
    ** Column (output)
    ** 1   2   3   4   5   6   7   8   9   10  11  12  13  14
    ** PF7 PF6 PF5 PF4 PF1 PF0 PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
    **
    ** Row (input)
    ** 1   2   3   4   5
    ** PD0 PD1 PD2 PD3 PD4
    */

    DDRF = 0b11110011;
    DDRB = 0b11111111;
    DDRD = 0b00000000;
}

volatile uint8_t * const matrix_col_port[COLUMN_COUNT] = {
    [0] = &PORTF,
    [1] = &PORTF,
    [2] = &PORTF,
    [3] = &PORTF,
    [4] = &PORTF,
    [5] = &PORTF,
    [6] = &PORTB,
    [7] = &PORTB,
    [8] = &PORTB,
    [9] = &PORTB,
    [10] = &PORTB,
    [11] = &PORTB,
    [12] = &PORTB,
    [13] = &PORTB,
};

const uint8_t matrix_col_pin[COLUMN_COUNT] = {
    [0] = PORTF7,
    [1] = PORTF6,
    [2] = PORTF5,
    [3] = PORTF4,
    [4] = PORTF1,
    [5] = PORTF0,
    [6] = PORTB0,
    [7] = PORTB1,
    [8] = PORTB2,
    [9] = PORTB3,
    [10] = PORTB4,
    [11] = PORTB5,
    [12] = PORTB6,
    [13] = PORTB7,
};

bool is_mod_key(uint8_t keycode) {
    return keycode >= 0xE0 && keycode <= 0xE7;
}

bool is_pressed(const uint8_t row_array[], uint8_t i, uint8_t j) {
    return row_array[i] & (1 << j);
}

int main(void) {
    LED_PROVE_INIT;
    matrix_init();
    usb_init();

    sei();

    for (;;) {
        for (uint8_t i = 0; i < COLUMN_COUNT; ++i) {
            *matrix_col_port[i] = _BV(matrix_col_pin[i]);
            _NOP();
            _NOP();
            buffer[i] = PIND;
            *matrix_col_port[i] = 0x00;
            _NOP();
            _NOP();
        }

        uint8_t tmp_ep_data_buffer[sizeof(usb_ep_data_buffer)] = { 0x00, };

        for (uint8_t i = 0; i < COLUMN_COUNT; ++i) {
            for (uint8_t j = 0; j < ROW_COUNT; ++j) {
                if (is_mod_key(keymap[i][j])) {
                    if (is_pressed(buffer, i, j)) {
                        tmp_ep_data_buffer[0] |= _BV(keymap[i][j] & 0x07);
                    } else {
                        tmp_ep_data_buffer[0] &= ~_BV(keymap[i][j] & 0x07);
                    }
                } else {
                    if (is_pressed(buffer, i, j)) {
                        tmp_ep_data_buffer[1 + (keymap[i][j] / 8)] |= _BV(keymap[i][j] % 8);
                    } else {
                        tmp_ep_data_buffer[1 + (keymap[i][j] / 8)] &= ~_BV(keymap[i][j] % 8);
                    }
                }
            }
        }

        if (!usb_ep_data_ready) {
            for (uint8_t i = 0; i < sizeof(usb_ep_data_buffer); ++i) {
                if (tmp_ep_data_buffer[i] != usb_ep_data_buffer[i]) {
                    for (uint8_t j = 0; j < sizeof(usb_ep_data_buffer); ++j) {
                        usb_ep_data_buffer[j] = tmp_ep_data_buffer[j];
                    }
                    usb_ep_data_ready = true;
                    break;
                }
            }
        }

        _delay_ms(1.0);
    }
}

