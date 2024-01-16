#ifndef UTILITY_H
#define UTILITY_H

#include <avr/io.h>
#include <util/delay.h>

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

#ifndef NDEBUG
#define LED_PROVE_INIT do { DDRC |= _BV(DDC7); } while (false)
#define LED_PROVE_HERE do { PORTC |= _BV(PORT7); _delay_ms(150); PORTC &= ~_BV(PORT7); _delay_ms(150); PORTC |= _BV(PORT7); } while (false)
#define LED_PROVE_SPARK do { PORTC |= _BV(PORT7); _delay_ms(1); PORTC &= ~_BV(PORT7); } while (false)
#else
#define LED_PROVE_INIT ((void)0)
#define LED_PROVE_HERE ((void)0)
#endif

#endif
