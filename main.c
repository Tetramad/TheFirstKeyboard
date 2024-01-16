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
    { KEY_F6, KEY_6, KEY_LEFTCTRL, KEY_SPACE, KEY_APOSTROPHE },
    { KEY_F7, KEY_7, KEY_RIGHTCTRL, KEY_RIGHTBRACE, KEY_LEFTBRACE },
    { KEY_F8, KEY_8, KEY_Y, KEY_H, KEY_N },
    { KEY_F9, KEY_9, KEY_U, KEY_J, KEY_M },
    { KEY_F10, KEY_0, KEY_I, KEY_K, KEY_COMMA },
    { KEY_F11, KEY_MINUS, KEY_O, KEY_L, KEY_DOT },
    { KEY_F12, KEY_EQUAL, KEY_P, KEY_SEMICOLON, KEY_SLASH },
    { KEY_RIGHTMETA, KEY_ESC, KEY_BACKSLASH, KEY_ENTER, KEY_RIGHTSHIFT },
};
uint8_t report_buffer[1 + (((164 + 4) + 7) / 8)] = { 0, };
bool changed = true;

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

void keymatrix_init(void) {
    /*
    ** Key Matrix Configuration
    **
    ** Column
    ** 1 PF7
    ** 2 PF6
    ** 3 PF5
    ** 4 PF4
    ** 5 PF1
    ** 6 PF0
    ** 7 PB0
    ** 8 PB1
    ** 9 PB2
    ** 10 PB3
    ** 11 PB4
    ** 12 PB5
    ** 13 PB6
    ** 14 PB7
    **
    ** Row
    ** 1 PD0
    ** 2 PD1
    ** 3 PD2
    ** 4 PD3
    ** 5 PD4
    */

    DDRF = 0b11110011;
    DDRB = 0b11111111;
    DDRD = 0b00000000;
}

int main(void) {
    LED_PROVE_INIT;
    usb_init();
    keymatrix_init();

    sei();

    for (;;) {
        PORTF |= _BV(PORTF7);
        _NOP();
        _NOP();
        buffer[0] = PIND;
        PORTF = 0x00;
        _NOP();
        _NOP();

        PORTF |= _BV(PORTF6);
        _NOP();
        _NOP();
        buffer[1] = PIND;
        PORTF = 0x00;
        _NOP();
        _NOP();

        PORTF |= _BV(PORTF5);
        _NOP();
        _NOP();
        buffer[2] = PIND;
        PORTF = 0x00;
        _NOP();
        _NOP();

        PORTF |= _BV(PORTF4);
        _NOP();
        _NOP();
        buffer[3] = PIND;
        PORTF = 0x00;
        _NOP();
        _NOP();

        PORTF |= _BV(PORTF1);
        _NOP();
        _NOP();
        buffer[4] = PIND;
        PORTF = 0x00;
        _NOP();
        _NOP();

        PORTF |= _BV(PORTF0);
        _NOP();
        _NOP();
        buffer[5] = PIND;
        PORTF = 0x00;
        _NOP();
        _NOP();

        PORTB |= _BV(PORTB0);
        _NOP();
        _NOP();
        buffer[6] = PIND;
        PORTB = 0x00;
        _NOP();
        _NOP();

        PORTB |= _BV(PORTB1);
        _NOP();
        _NOP();
        buffer[7] = PIND;
        PORTB = 0x00;
        _NOP();
        _NOP();

        PORTB |= _BV(PORTB2);
        _NOP();
        _NOP();
        buffer[8] = PIND;
        PORTB = 0x00;
        _NOP();
        _NOP();

        PORTB |= _BV(PORTB3);
        _NOP();
        _NOP();
        buffer[9] = PIND;
        PORTB = 0x00;
        _NOP();
        _NOP();

        PORTB |= _BV(PORTB4);
        _NOP();
        _NOP();
        buffer[10] = PIND;
        PORTB = 0x00;
        _NOP();
        _NOP();

        PORTB |= _BV(PORTB5);
        _NOP();
        _NOP();
        buffer[11] = PIND;
        PORTB = 0x00;
        _NOP();
        _NOP();

        PORTB |= _BV(PORTB6);
        _NOP();
        _NOP();
        buffer[12] = PIND;
        PORTB = 0x00;
        _NOP();
        _NOP();

        PORTB |= _BV(PORTB7);
        _NOP();
        _NOP();
        buffer[13] = PIND;
        PORTB = 0x00;
        _NOP();
        _NOP();

#define IS_MOD_KEY(keycode) ((keycode) >= 0xE0 && (keycode) <= 0xE7)
#define KEYCODE_TO_MODFLAG(keycode) (1 << ((keycode) & 0x07))
#define ACTIVE_KEY(keycode) do { if (IS_MOD_KEY(keycode)) { report_buffer[0] |= KEYCODE_TO_MODFLAG(keycode); } else { report_buffer[(keycode / 8) + 1] |= 1 << (keycode % 8); } } while (false)
#define DEACTIVE_KEY(keycode) do { if (IS_MOD_KEY(keycode)) { report_buffer[0] &= ~KEYCODE_TO_MODFLAG(keycode); } else { report_buffer[(keycode / 8) + 1] &= ~(1 << (keycode % 8)); } } while (false)

        bool tmp_changed = false;
        for (uint8_t i = 0; i < COLUMN_COUNT; ++i) {
            for (uint8_t j = 0; j < ROW_COUNT; ++j) {
                if (IS_MOD_KEY(keymap[i][j])) {
                    if ((report_buffer[0] ^ ((!!(buffer[i] & (1 << j))) << (keymap[i][j] & 0x07))) & (1 << (keymap[i][j] & 0x07))) {
                        tmp_changed = true;
                    }
                } else {
                    if ((report_buffer[1 + (keymap[i][j] / 8)] ^ ((!!(buffer[i] & (1 << j))) << (keymap[i][j] % 8))) & (1 << (keymap[i][j] % 8))) {
                        tmp_changed = true;
                    }
                }

                if (buffer[i] & (1 << j)) {
                    ACTIVE_KEY(keymap[i][j]);
                } else {
                    DEACTIVE_KEY(keymap[i][j]);
                }

                changed = tmp_changed;
            }
        }
        _delay_ms(1.0);
    }
}

