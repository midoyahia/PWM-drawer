/**
 * @file DIO.c
 * @brief Decode the port and bit number, then access the selected GPIO register.
 */
#include "DIO.h"

/**
 * @copydoc DIO_set_pin
 */
void DIO_set_pin(GPIO_pin_t pin){
    volatile uint8_t * port  =  GPIO_GetPORT_FromPin(pin);
    SET_BIT(*port,GET_LOWER_NIBBLE(pin));
}
/**
 * @copydoc DIO_clear_pin
 */
void DIO_clear_pin(GPIO_pin_t pin){
    volatile uint8_t * port  =  GPIO_GetPORT_FromPin(pin);
    CLEAR_BIT(*port,GET_LOWER_NIBBLE(pin));   
}
/**
 * @copydoc DIO_toggle_pin
 */
void DIO_toggle_pin(GPIO_pin_t pin){
        volatile uint8_t * port  =  GPIO_GetPORT_FromPin(pin);
    TOGGLE_BIT(*port,GET_LOWER_NIBBLE(pin));   
}
/**
 * @copydoc DIO_read_pin
 */
DIO_boot_t DIO_read_pin(GPIO_pin_t pin){
    volatile uint8_t * pin_reg =  GPIO_GetPIN_FromPin(pin);
    return READ_BIT(*pin_reg,GET_LOWER_NIBBLE(pin)); 
}
/**
 * @copydoc DIO_is_Set
 */
DIO_boot_t DIO_is_Set(GPIO_pin_t pin){
    volatile uint8_t * port  =  GPIO_GetPORT_FromPin(pin);
    return READ_BIT(*port,GET_LOWER_NIBBLE(pin));   
}
/**
 * @copydoc DIO_write_pin
 */
void DIO_write_pin(GPIO_pin_t pin,DIO_boot_t value){
       volatile uint8_t * port  =  GPIO_GetPORT_FromPin(pin);
    WRITE_BIT(*port,GET_LOWER_NIBBLE(pin),value); 
}
