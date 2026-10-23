#ifndef FONT_H
#define FONT_H

#include <stdint.h>

/**
 * @brief 5x7 ASCII font table (characters 32–127).
 * Each character is 5 bytes wide, each byte is one vertical column of 8 pixels.
 * Bit 0 = top pixel, Bit 7 = bottom pixel.
 */
extern const uint8_t font5x7[][5];

#endif /* FONT_H */
