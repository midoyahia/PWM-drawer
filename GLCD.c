/**
 * @file GLCD.c
 * @brief KS0108 128x64 graphical LCD driver implementation 
 */
#include "GLCD.h"
#include "FONT.h"
#include "../../MCAL/DIO/DIO.h"
#include "../../CONFIG.h"
#include <util/delay.h>

#define CMD_DISPLAY_ON   0x3F
#define CMD_START_LINE   0xC0
#define CMD_SET_PAGE     0xB8
#define CMD_SET_COLUMN   0x40

/** @brief Drive a chip-select pin honouring GLCD_CS_ACTIVE_HIGH. */
static void cs_write(GPIO_pin_t pin, uint8_t active)
{
#if GLCD_CS_ACTIVE_HIGH
    DIO_write_pin(pin, active ? DIO_HIGH : DIO_LOW);
#else
    DIO_write_pin(pin, active ? DIO_LOW : DIO_HIGH);
#endif
}

/** @brief Select the left (0) or right (1) half; the other half ignores bus traffic. */
static void select_chip(uint8_t chip)
{
    cs_write(GLCD_CS1, chip == 0);
    cs_write(GLCD_CS2, chip == 1);
}

/** @brief Select the chip owning column x and point it at (page, x). */
static void set_xy(uint8_t page, uint8_t x)
{
    select_chip(x >> 6);
    GLCD_SendCommand(CMD_SET_PAGE | (page & 7));
    GLCD_SendCommand(CMD_SET_COLUMN | (x & 63));
}
 
void GLCD_Enable(void)
{
    DIO_set_pin(GLCD_EN);
    _delay_us(1);
    DIO_clear_pin(GLCD_EN);
    _delay_us(GLCD_E_DELAY_US);
}

/** @brief Put a byte on D0..D7. */
static void put_bus(uint8_t v)
{
    for (uint8_t i = 0; i < 8; i++) {
        DIO_write_pin((GPIO_pin_t)(GLCD_D0 + i), (v >> i) & 1 ? DIO_HIGH : DIO_LOW);
    }
}

void GLCD_SendCommand(uint8_t cmd)
{
    DIO_clear_pin(GLCD_RS);
    put_bus(cmd);
    GLCD_Enable();
}

void GLCD_SendData(uint8_t data)
{
    DIO_set_pin(GLCD_RS);
    put_bus(data);
    GLCD_Enable();
}

void GLCD_Init(void)
{
    GPIO_Pin_Mode(GLCD_RS,  GPIO_DIO_OUTPUT);
    GPIO_Pin_Mode(GLCD_EN,  GPIO_DIO_OUTPUT);
    GPIO_Pin_Mode(GLCD_CS1, GPIO_DIO_OUTPUT);
    GPIO_Pin_Mode(GLCD_CS2, GPIO_DIO_OUTPUT);
    GPIO_Pin_Mode(GLCD_RST, GPIO_DIO_OUTPUT);
    for (uint8_t i = 0; i < 8; i++) {
        GPIO_Pin_Mode((GPIO_pin_t)(GLCD_D0 + i), GPIO_DIO_OUTPUT);
    }

    DIO_clear_pin(GLCD_EN);
    DIO_clear_pin(GLCD_RST);          /* hardware reset pulse */
    _delay_ms(2);
    DIO_set_pin(GLCD_RST);
    _delay_ms(20);

    for (uint8_t chip = 0; chip < 2; chip++) {
        select_chip(chip);
        GLCD_SendCommand(CMD_DISPLAY_ON);
        GLCD_SendCommand(CMD_START_LINE);
    }
    GLCD_Clear();
}

void GLCD_WritePage(uint8_t page, const uint8_t *buf)
{
    for (uint8_t chip = 0; chip < 2; chip++) {
        set_xy(page, (uint8_t)(chip << 6));
        for (uint8_t i = 0; i < 64; i++) {
            GLCD_SendData(buf[(chip << 6) + i]);
        }
    }
}

void GLCD_Clear(void)
{
    for (uint8_t page = 0; page < GLCD_PAGES; page++) {
        for (uint8_t chip = 0; chip < 2; chip++) {
            set_xy(page, (uint8_t)(chip << 6));
            for (uint8_t i = 0; i < 64; i++) {
                GLCD_SendData(0x00);
            }
        }
    }
}

/** @brief Column byte i (0..4 glyph, 5 = spacing) of a printable character. */
static uint8_t glyph_col(char c, uint8_t i)
{
    if (c < 32 || c > 126 || i > 4) return 0x00;
    return font5x7[c - 32][i];
}

void GLCD_PrintChar(char c, uint8_t page, uint8_t x)
{
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t col = (uint8_t)(x + i);
        if (col >= GLCD_WIDTH) return;
        /* re-address at the start and whenever we cross into the right chip */
        if (i == 0 || (col & 63) == 0) set_xy(page, col);
        GLCD_SendData(glyph_col(c, i));
    }
}

void GLCD_PrintString(const char *str, uint8_t page, uint8_t x)
{
    while (*str && x < GLCD_WIDTH) {
        GLCD_PrintChar(*str++, page, x);
        x += 6;
    }
}

/* ------------------------- page-buffer helpers ------------------------- */

void GLCD_BufClear(uint8_t *buf)
{
    for (uint8_t i = 0; i < GLCD_WIDTH; i++) buf[i] = 0;
}

void GLCD_BufText(uint8_t *buf, const char *str, uint8_t x)
{
    while (*str) {
        for (uint8_t i = 0; i < 5; i++) {
            if (x + i < GLCD_WIDTH) buf[x + i] |= glyph_col(*str, i);
        }
        str++;
        x += 6;
        if (x >= GLCD_WIDTH) break;
    }
}

void GLCD_BufPixel(uint8_t *buf, uint8_t page, uint8_t x, uint8_t y)
{
    if (x < GLCD_WIDTH && (y >> 3) == page) buf[x] |= (uint8_t)(1 << (y & 7));
}

void GLCD_BufHLine(uint8_t *buf, uint8_t page, uint8_t x1, uint8_t x2, uint8_t y)
{
    if ((y >> 3) != page) return;
    uint8_t mask = (uint8_t)(1 << (y & 7));
    if (x1 > x2) { uint8_t t = x1; x1 = x2; x2 = t; }
    for (uint8_t x = x1; x <= x2 && x < GLCD_WIDTH; x++) buf[x] |= mask;
}

void GLCD_BufVLine(uint8_t *buf, uint8_t page, uint8_t x, uint8_t y1, uint8_t y2)
{
    if (y1 > y2) { uint8_t t = y1; y1 = y2; y2 = t; }
    for (uint8_t y = y1; y <= y2; y++) GLCD_BufPixel(buf, page, x, y);
}
