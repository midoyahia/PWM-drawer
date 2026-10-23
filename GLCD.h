/**
 * @file GLCD.h
 * @brief KS0108 128x64 graphical LCD driver (two 64-column halves, write-only).
 *
 * Wiring (R/W must be tied to GND, V0 to the contrast pot):
 *   D0..D7 -> PA0..PA7     RS -> PB0     E -> PB2
 *   CS1 (left half, x 0..63)   -> PC0
 *   CS2 (right half, x 64..127)-> PC1
 *   RST -> PC6
 * PC2..PC5 are avoided on purpose: the ATmega32 JTAG fuse is enabled by default.
 *
 * Memory layout: 8 pages of 8 pixel rows; in a page byte bit0 is the top pixel.
 */
#ifndef GLCD_H
#define GLCD_H

#include "../../MCAL/GPIO/GPIO.h"
#include "../../MCAL/DIO/DIO.h"
#include <stdint.h>

#define GLCD_RS   GPIO_PB0
#define GLCD_EN   GPIO_PB2
#define GLCD_CS1  GPIO_PC1
#define GLCD_CS2  GPIO_PC0
#define GLCD_RST  GPIO_PC6
#define GLCD_D0   GPIO_PA0
#define GLCD_D1   GPIO_PA1
#define GLCD_D2   GPIO_PA2
#define GLCD_D3   GPIO_PA3
#define GLCD_D4   GPIO_PA4
#define GLCD_D5   GPIO_PA5
#define GLCD_D6   GPIO_PA6
#define GLCD_D7   GPIO_PA7

/** @brief 1 if the chip-select lines are active high (most KS0108 modules), 0 if active low. */
#define GLCD_CS_ACTIVE_HIGH  1
/** @brief Delay after each E pulse; lower it for speed, raise it if the display shows garbage. */
#define GLCD_E_DELAY_US      10

#define GLCD_WIDTH   128
#define GLCD_PAGES   8

/* ---- direct (immediate) access ---- */
void GLCD_Init(void);
void GLCD_Clear(void);
void GLCD_SendCommand(uint8_t cmd);
void GLCD_SendData(uint8_t data);
void GLCD_Enable(void);
/** @brief Print one 5x7 character (6 px cell) at pixel column x (0..127) of a page. */
void GLCD_PrintChar(char c, uint8_t page, uint8_t x);
/** @brief Print a string at pixel column x; characters may cross the 64-column chip boundary. */
void GLCD_PrintString(const char *str, uint8_t page, uint8_t x);
/** @brief Write a full 128-byte page image. */
void GLCD_WritePage(uint8_t page, const uint8_t *buf);

/* ---- page-buffer helpers: compose one page in RAM, then GLCD_WritePage() ---- */
void GLCD_BufClear(uint8_t *buf);
/** @brief Draw text into a page buffer; bit0 of each column is the top pixel row of the page. */
void GLCD_BufText(uint8_t *buf, const char *str, uint8_t x);
/** @brief Set pixel (x, y) with y in absolute rows 0..63; ignored if y is outside @p page. */
void GLCD_BufPixel(uint8_t *buf, uint8_t page, uint8_t x, uint8_t y);
/** @brief Horizontal line x1..x2 at absolute row y (clipped to @p page). */
void GLCD_BufHLine(uint8_t *buf, uint8_t page, uint8_t x1, uint8_t x2, uint8_t y);
/** @brief Vertical line at x from absolute row y1..y2 (clipped to @p page). */
void GLCD_BufVLine(uint8_t *buf, uint8_t page, uint8_t x, uint8_t y1, uint8_t y2);

#endif
