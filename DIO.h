/**
 * @file DIO.h
 * @brief Digital input/output helpers using encoded GPIO pin identifiers.
 * @pre Use a valid GPIO_PA0..GPIO_PD7 identifier and configure its direction
 * with GPIO_Pin_Mode(). These helpers do not validate pins or set direction.
 * @note Writes operate on PORT latches; reads of electrical levels use PIN.
 * @warning Register read-modify-write operations are not protected against
 * concurrent updates from interrupts to other bits of the same port.
 */
#ifndef DIO_H
#define DIO_H


#include "../GPIO/GPIO.h"

/**
 * @brief Boolean GPIO level returned or accepted by DIO operations.
 */
typedef enum {
    DIO_LOW,
    DIO_HIGH
}DIO_boot_t;
/**
 * @brief Set the pin output latch high.
 * @param pin Encoded port and pin identifier.
 * @note On an input, this requests the internal pull-up instead of driving high.
 */
void DIO_set_pin(GPIO_pin_t pin);
/**
 * @brief Set or clear a pin latch according to a Boolean value.
 * @param pin Encoded port and pin identifier.
 * @param value DIO_LOW clears the bit; a nonzero value sets it.
 */
void DIO_write_pin(GPIO_pin_t pin,DIO_boot_t value);
/**
 * @brief Clear the pin output latch.
 * @param pin Encoded port and pin identifier.
 * @note On an input, this disables its port-controlled pull-up.
 */
void DIO_clear_pin(GPIO_pin_t pin);
/**
 * @brief Invert the current PORT latch bit.
 * @param pin Encoded port and pin identifier.
 */
void DIO_toggle_pin(GPIO_pin_t pin);
/**
 * @brief Sample the electrical input level using PIN.
 * @param pin Encoded port and pin identifier.
 * @return DIO_LOW or DIO_HIGH.
 */
DIO_boot_t DIO_read_pin(GPIO_pin_t pin);
/**
 * @brief Read the stored PORT latch state.
 * @param pin Encoded port and pin identifier.
 * @return DIO_HIGH if the latch bit is set, otherwise DIO_LOW.
 * @note This is not a measurement of the electrical input level.
 */
DIO_boot_t DIO_is_Set(GPIO_pin_t pin);

#endif
