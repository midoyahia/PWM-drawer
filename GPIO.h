/**
 * @file GPIO.h
 * @brief General-purpose input/output interface for AVR ports.
 *
 * This module wraps the AVR DDR, PORT, and PIN registers with typed pin and
 * state values. Register addresses are passed by pointer so the same functions
 * can operate on any available hardware port.
 * @pre Register pointers must be valid and bit indices must be in 0..7.
 * @warning Port updates are read-modify-write operations without interrupt
 * protection. Coordinate callers that share a register.
 */
#ifndef _GPIO_H_
#define _GPIO_H_
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../../TOOLS.h"


/**
 * @brief Pin encoding: upper nibble is port index, lower nibble is bit 0..7.
 */
typedef enum {
    GPIO_PA0 = 0x00,
    GPIO_PA1 = 0x01,
    GPIO_PA2 = 0x02,
    GPIO_PA3 = 0x03,
    GPIO_PA4 = 0x04,
    GPIO_PA5 = 0x05,
    GPIO_PA6 = 0x06,
    GPIO_PA7 = 0x07,
    GPIO_PB0 = 0x10,
    GPIO_PB1 = 0x11,
    GPIO_PB2 = 0x12,
    GPIO_PB3 = 0x13,
    GPIO_PB4 = 0x14,
    GPIO_PB5 = 0x15,
    GPIO_PB6 = 0x16,
    GPIO_PB7 = 0x17, 
    GPIO_PC0 = 0x20,
    GPIO_PC1 = 0x21,
    GPIO_PC2 = 0x22,
    GPIO_PC3 = 0x23,
    GPIO_PC4 = 0x24,
    GPIO_PC5 = 0x25,
    GPIO_PC6 = 0x26,
    GPIO_PC7 = 0x27,
    GPIO_PD0 = 0x30,
    GPIO_PD1 = 0x31,
    GPIO_PD2 = 0x32,
    GPIO_PD3 = 0x33,
    GPIO_PD4 = 0x34,
    GPIO_PD5 = 0x35,
    GPIO_PD6 = 0x36,
    GPIO_PD7 = 0x37
}GPIO_pin_t;
/**
 * @brief Port indices used for PORT register lookup.
 */
typedef enum {
    GPIO_PORTA = 0x00,
    GPIO_PORTB = 0x01,
    GPIO_PORTC = 0x02,
    GPIO_PORTD = 0x03
}GPIO_port_reg_t;
/**
 * @brief Port indices used for DDR register lookup.
 */
typedef enum {
    GPIO_DDRA = 0x00,
    GPIO_DDRB = 0x01,
    GPIO_DDRC = 0x02,
    GPIO_DDRD = 0x03
}GPIO_DDR_reg_t;
/**
 * @brief Port indices used for PIN register lookup.
 */
typedef enum {
    GPIO_PINA = 0x00,
    GPIO_PINB = 0x01,
    GPIO_PINC = 0x02,
    GPIO_PIND = 0x03
}GPIO_PIN_reg_t;

/**
 * @brief Pin direction and pull-up configuration choices.
 */
typedef enum {
    GPIO_DIO_OUTPUT,
    GPIO_DIO_INPUT,
    GPIO_DIO_INPUT_PULLUP,
    GPIO_ADC_INPUT,
    
}GPIO_pin_mode_t;

/**
 * @brief Configure direction and optional input pull-up.
 * @param pin Valid encoded port/pin identifier.
 * @param mode Requested pin mode; unknown modes leave registers unchanged.
 * @details Output mode preserves the PORT latch. Plain input clears the latch.
 * @warning GPIO_ADC_INPUT currently follows INPUT_PULLUP and sets PORT high;
 * it does not configure a floating analog input. Invalid pins are unchecked.
 */
void GPIO_Pin_Mode (GPIO_pin_t pin ,GPIO_pin_mode_t mode  );

/**
 * @brief Resolve the sampled input register for an encoded pin.
 * @param pin Encoded identifier; the upper nibble selects port A, B, C, or D.
 * @return Register address, or a null pointer for an unknown port nibble.
 * @note The lower nibble is not validated by this lookup.
 */
volatile uint8_t * GPIO_GetPIN_FromPin(GPIO_pin_t pin);
/**
 * @brief Resolve the output latch register for an encoded pin.
 * @param pin Encoded identifier; the upper nibble selects port A, B, C, or D.
 * @return Register address, or a null pointer for an unknown port nibble.
 * @note The lower nibble is not validated by this lookup.
 */
volatile uint8_t * GPIO_GetPORT_FromPin(GPIO_pin_t pin);
/**
 * @brief Resolve the data-direction register for an encoded pin.
 * @param pin Encoded identifier; the upper nibble selects port A, B, C, or D.
 * @return Register address, or a null pointer for an unknown port nibble.
 * @note The lower nibble is not validated by this lookup.
 */
volatile uint8_t * GPIO_GetDDR_FromPin(GPIO_pin_t pin);






























/** @brief Selects whether a GPIO pin is configured as an input or output. */
typedef enum {
GPIO_INPUT = 0 , /**< Clear the DDR bit and configure the pin as an input. */
GPIO_OUTPUT     /**< Set the DDR bit and configure the pin as an output. */
}GPIO_direction_t;

/** @brief Identifies a bit position within an eight-bit AVR GPIO port. */
typedef enum {
GPIO_PIN0 = 0,
GPIO_PIN1,
GPIO_PIN2,
GPIO_PIN3,
GPIO_PIN4,
GPIO_PIN5,
GPIO_PIN6,
GPIO_PIN7
}GPIO_PIN_t;

/** @brief Logical voltage level read from or written to a GPIO pin. */
typedef enum {
GPIO_LOW = 0 , /**< Logic-low state. */
GPIO_HIGH     /**< Logic-high state. */
}GPIO_Pin_State_t;


/** @brief Identifies an AVR external interrupt source. */
typedef enum {
    GPIO_INT0 ,
    GPIO_INT1,
    GPIO_INT2
}GPIO_Int_t;

/** @brief Selects the electrical event that activates an external interrupt. */
typedef enum {
GPIO_RISING_EDGE ,
GPIO_FALLING_EDGE,
GPIO_LOGIC_CHANGE,
GPIO_LOW_LVL_INT
}GPIO_Int_type_t;


/** @brief Function type used for interrupt callback handlers. */
typedef void (*GPIO_Callback_t)(void);

/**
 * @brief Configure the direction of one GPIO pin.
 * @param ddr Address of the port's data-direction register.
 * @param pin Bit position of the pin to configure.
 * @param direction Required input or output direction.
 */
void GPIO_Pin_Direction (volatile uint8_t * ddr ,GPIO_PIN_t pin ,GPIO_direction_t direction  );
/**
 * @brief Drive one GPIO output pin high or low.
 * @param port Address of the port output register.
 * @param pin Bit position of the pin to update.
 * @param value Zero drives low; any nonzero value drives high.
 */
void GPIO_Pin_Write (volatile uint8_t * port ,GPIO_PIN_t pin ,uint8_t value  );
/**
 * @brief Enable the AVR internal pull-up resistor on an input pin.
 * @param port Address of the port output register.
 * @param pin Bit position of the input pin.
 */
void GPIO_Set_PullUp (volatile uint8_t * port ,GPIO_PIN_t pin  );
/**
 * @brief Disable the AVR internal pull-up resistor on an input pin.
 * @param port Address of the port output register.
 * @param pin Bit position of the input pin.
 */
void GPIO_Clear_PullUp (volatile uint8_t * port ,GPIO_PIN_t pin  );
/**
 * @brief Toggle the current output state of one GPIO pin.
 * @warning Declared here, but no GPIO_Pin_Toggle implementation exists in
 * GPIO.c. Calling it requires an implementation or produces a link error.
 * @param port Address of the port output register.
 * @param pin Bit position of the pin to toggle.
 */
void GPIO_Pin_Toggle (volatile uint8_t * port ,GPIO_PIN_t pin );
/**
 * @brief Read the sampled electrical state of one GPIO pin.
 * @param pin_reg Address of the port input register.
 * @param pin_no Bit position of the pin to sample.
 * @return GPIO_LOW or GPIO_HIGH.
 */
GPIO_Pin_State_t GPIO_Pin_Read (volatile uint8_t * pin_reg ,GPIO_PIN_t pin_no  );
/**
 * @brief Set one PORT latch bit.
 * @param port Non-null pointer to the output register.
 * @param pin Bit index from GPIO_PIN0 through GPIO_PIN7.
 */
void GPIO_Pin_Set (volatile uint8_t * port ,GPIO_PIN_t pin );
/**
 * @brief Clear one PORT latch bit.
 * @param port Non-null pointer to the output register.
 * @param pin Bit index from GPIO_PIN0 through GPIO_PIN7.
 */
void GPIO_Pin_Clear (volatile uint8_t * port ,GPIO_PIN_t pin );

#endif /* _GPIO_H_ */
