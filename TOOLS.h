/**
 * @file TOOLS.h
 * @brief Common bit-manipulation utilities for AVR registers.
 *
 * 
 * The macros in this file operate directly on register-like lvalues. They are
 * shared by the MCAL drivers to keep register operations short and readable.
 * @pre REG must be a suitable integer expression, or a writable lvalue for
 * modifying macros. Use valid bit positions (0..7 for eight-bit registers).
 * @warning Writes are read-modify-write operations, not general atomic
 * operations. Special registers with write-one-to-clear flags need care.
 * Avoid arguments with side effects; these macros are not type-safe functions.
 */
#ifndef TOOLS_H
#define TOOLS_H

#include <stdint.h>

/**
 * @brief Set one bit.
 * @param REG Register lvalue or integer expression.
 * @param BIT Zero-based bit index.
 */
#define SET_BIT(REG,BIT)                ((REG)|=(1<<(BIT)))
/**
 * @brief Clear one bit.
 * @param REG Register lvalue or integer expression.
 * @param BIT Zero-based bit index.
 */
#define CLEAR_BIT(REG,BIT)              ((REG)&=~(1<<(BIT)))
/**
 * @brief Write one Boolean bit.
 * @param REG Register lvalue or integer expression.
 * @param BIT Zero-based bit index.
 * @param VALUE Zero clears the bit; nonzero sets it.
 */
#define WRITE_BIT(REG,BIT,VALUE)        ((VALUE)? SET_BIT(REG,BIT):CLEAR_BIT(REG,BIT))
/**
 * @brief Extract one bit as zero or one.
 * @param REG Register lvalue or integer expression.
 * @param BIT Zero-based bit index.
 * @return Extracted unsigned bit field.
 */
#define READ_BIT( REG , BIT )           (((REG) >> (BIT)) & 0x01)                                                 
/**
 * @brief Invert one bit.
 * @param REG Register lvalue or integer expression.
 * @param BIT Zero-based bit index.
 */
#define TOGGLE_BIT( REG , BIT )         ((REG)^=(1<<(BIT)))

/**
 * @brief Extract bits 3:0 as a value in 0..15.
 * @param REG Register lvalue or integer expression.
 * @return Extracted unsigned bit field.
 */
#define GET_LOWER_NIBBLE(REG)            ((REG) & 0x0F)
/**
 * @brief Extract bits 7:4 and shift them down to a value in 0..15.
 * @param REG Register lvalue or integer expression.
 * @return Extracted unsigned bit field.
 */
#define GET_UPPER_NIBBLE(REG)             (((REG) & 0xF0)>>4)

/**
 * @brief Map a value from one range to another.
 * @param x Input value to be scaled.
 * @param x1 Lower bound of the source range.
 * @param x2 Upper bound of the source range.
 * @param y1 Lower bound of the destination range.
 * @param y2 Upper bound of the destination range.
 * @return Value mapped proportionally across the new range.
 * @note Uses signed 32-bit intermediate arithmetic; division truncates.
 * @pre x2 must differ from x1 and all intermediates must fit int32_t.
 */

#define MAP(x, x1, x2, y1, y2) \
 ((int32_t)(y1) + ((int32_t)(x) - (int32_t)(x1)) * \
 ((int32_t)(y2) - (int32_t)(y1)) / ((int32_t)(x2) - (int32_t)(x1)))

#endif /* TOOLS_H */
