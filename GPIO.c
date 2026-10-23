/**
 * @file GPIO.c
 * @brief Implementation of the AVR GPIO abstraction.
 */
#include "GPIO.h"
#include "../../TOOLS.h"

/**
 * @copydoc GPIO_GetPIN_FromPin
 */
volatile uint8_t * GPIO_GetPIN_FromPin(GPIO_pin_t  pin){
switch (GET_UPPER_NIBBLE(pin)){
    case GPIO_PINA :
        return &PINA;
        break;
    
    case GPIO_PINB :
        return &PINB;
        break;
    
    case GPIO_PINC :
        return &PINC;
        break;
    
    case GPIO_PIND :
        return &PIND;
        break;
}
    return 0;
}
/**
 * @copydoc GPIO_GetPORT_FromPin
 */
volatile uint8_t * GPIO_GetPORT_FromPin(GPIO_pin_t pin){

    switch (GET_UPPER_NIBBLE(pin)){
    case GPIO_PORTA :
        return &PORTA;
        break;
    
    case GPIO_PORTB :
        return &PORTB;
        break;
    
    case GPIO_PORTC :
        return &PORTC;
        break;
    
    case GPIO_PORTD :
        return &PORTD;
        break;
}
    return 0;

}
/**
 * @copydoc GPIO_GetDDR_FromPin
 */
volatile uint8_t * GPIO_GetDDR_FromPin(GPIO_pin_t pin){
    switch (GET_UPPER_NIBBLE(pin)){
    case GPIO_DDRA :
        return &DDRA;
        break;
    
    case GPIO_DDRB :
        return &DDRB;
        break;
    
    case GPIO_DDRC :
        return &DDRC;
        break;
    
    case GPIO_DDRD :
        return &DDRD;
        break;
}
    return 0;
}



/**
 * @copydoc GPIO_Pin_Mode
 */
void GPIO_Pin_Mode (GPIO_pin_t pin ,GPIO_pin_mode_t mode  ){
    volatile uint8_t * ddr  = GPIO_GetDDR_FromPin(pin);
    volatile uint8_t * port  = GPIO_GetPORT_FromPin(pin);
    switch (mode)
    {
    case GPIO_DIO_OUTPUT:
        SET_BIT(* ddr , GET_LOWER_NIBBLE(pin));
        break;
    
    case GPIO_DIO_INPUT:
        CLEAR_BIT(* ddr , GET_LOWER_NIBBLE(pin));
        CLEAR_BIT(* port  , GET_LOWER_NIBBLE(pin));
        break;
    
    case GPIO_DIO_INPUT_PULLUP:
    case GPIO_ADC_INPUT:
        CLEAR_BIT(* ddr , GET_LOWER_NIBBLE(pin));
        SET_BIT(* port  , GET_LOWER_NIBBLE(pin));
        break;
    default:
        break;
    }
}



























/** @copydoc GPIO_Pin_Direction */
void GPIO_Pin_Direction (volatile uint8_t * ddr ,GPIO_PIN_t pin ,GPIO_direction_t direction  ){
 /* AVR DDR bits use 0 for input and 1 for output. */
 WRITE_BIT(*ddr,pin,direction);
}
/** @copydoc GPIO_Pin_Write */
void GPIO_Pin_Write (volatile uint8_t * port ,GPIO_PIN_t pin ,uint8_t value  ){
    WRITE_BIT(*port,pin,value);
}

/**
 * @copydoc GPIO_Pin_Set
 */
void GPIO_Pin_Set (volatile uint8_t * port ,GPIO_PIN_t pin ){
   SET_BIT(*port,pin);
}
/**
 * @copydoc GPIO_Pin_Clear
 */
void GPIO_Pin_Clear (volatile uint8_t * port ,GPIO_PIN_t pin ){
 CLEAR_BIT(*port,pin);
}

/** @copydoc GPIO_Set_PullUp */
void GPIO_Set_PullUp (volatile uint8_t * port ,GPIO_PIN_t pin  ){
    /* An input's PORT bit enables its internal pull-up when set. */
    SET_BIT(*port,pin);
}
/** @copydoc GPIO_Clear_PullUp */
void GPIO_Clear_PullUp (volatile uint8_t * port ,GPIO_PIN_t pin  ){
    CLEAR_BIT(*port,pin);
}
/** @copydoc GPIO_Pin_Read */
GPIO_Pin_State_t GPIO_Pin_Read (volatile uint8_t * pin_reg ,GPIO_PIN_t pin_no  ){
    return READ_BIT(*pin_reg,pin_no);
}
