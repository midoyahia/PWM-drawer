/**
 * @file TIMER.c
 * @brief Timer0 and Timer1 driver implementation for the AVR ATmega32A.
 */
#include "TIMER.h"
#include "../../TOOLS.h"

#include <avr/interrupt.h>
#include <avr/io.h>

/** @brief Timer0 ISR callbacks; null values suppress application dispatch.
 * @details Volatile pointers are updated with interrupts temporarily disabled.
 */
static callback_t volatile timer0_overflow_callback;
static callback_t volatile timer0_compare_callback;

/** @brief Timer1 ISR callbacks; null values suppress application dispatch. */
static callback_t volatile timer1_overflow_callback;
static callback_t volatile timer1_compare_a_callback;
static callback_t volatile timer1_compare_b_callback;

/** @copydoc TIMER0_init */
void TIMER0_init(TIMER0_Mode_t mode,
                 TIMER0_compare_output_mode_t cmpr,
                 TIMER0_CLK_t clk)
{
    /* Clear the Timer0 configuration fields before applying new settings. */
    TCCR0 = 0;

    switch (mode) {
        case TIMER0_PHASECORRECT_PWM:
            SET_BIT(TCCR0, WGM00);
            break;

        case TIMER0_CTC_MODE:
            SET_BIT(TCCR0, WGM01);
            break;

        case TIMER0_FAST_PWM:
            SET_BIT(TCCR0, WGM01);
            SET_BIT(TCCR0, WGM00);
            break;

        case TIMER0_NORMAL_MODE:
        default:
            break;
    }

    switch (cmpr) {
        case TIMER0_NON_PWM_OCR0_TOGGLE:
            SET_BIT(TCCR0, COM00);
            break;

        case TIMER0_NON_PWM_OCR0_CLEAR:
        case TIMER0_FAST_PWM_OCR0_CLEAR_OC_SET_BOTTOM:
        case TIMER0_PHASE_PWM_OCR0_CLEAR_UP_SET_DOWN:
            SET_BIT(TCCR0, COM01);
            break;

        case TIMER0_NON_PWM_OCR0_SET:
        case TIMER0_FAST_PWM_OCR0_SET_OC_CLEAR_BOTTOM:
        case TIMER0_PHASE_PWM_OCR0_SET_UP_CLEAR_DOWN:
            SET_BIT(TCCR0, COM01);
            SET_BIT(TCCR0, COM00);
            break;

        case TIMER0_OCR0_DISCONNECTED:
        default:
            break;
    }

    switch (clk) {
        case TIMER0_NO_PRESCALER:
            SET_BIT(TCCR0, CS00);
            break;

        case TIMER0_PRESCALER_8:
            SET_BIT(TCCR0, CS01);
            break;

        case TIMER0_PRESCALER_64:
            SET_BIT(TCCR0, CS01);
            SET_BIT(TCCR0, CS00);
            break;

        case TIMER0_PRESCALER_256:
            SET_BIT(TCCR0, CS02);
            break;

        case TIMER0_PRESCALER_1024:
            SET_BIT(TCCR0, CS02);
            SET_BIT(TCCR0, CS00);
            break;

        case TIMER0_EXTERNAL_FALLING_EDGE:
            SET_BIT(TCCR0, CS02);
            SET_BIT(TCCR0, CS01);
            break;

        case TIMER0_EXTERNAL_RISING_EDGE:
            SET_BIT(TCCR0, CS02);
            SET_BIT(TCCR0, CS01);
            SET_BIT(TCCR0, CS00);
            break;

        case TIMER0_NO_CLK:
        default:
            break;
    }
}

/** @copydoc TIMER0_setOCR0 */
void TIMER0_setOCR0(uint8_t value)
{
    OCR0 = value;
}

/** @copydoc TIMER0_OVF_interrupt_init */
void TIMER0_OVF_interrupt_init(callback_t ptr)
{
    uint8_t interrupt_state = SREG;

    cli();
    timer0_overflow_callback = ptr;
    SREG = interrupt_state;
}

/** @copydoc TIMER0_OCR0_interrupt_init */
void TIMER0_OCR0_interrupt_init(callback_t ptr)
{
    uint8_t interrupt_state = SREG;

    cli();
    timer0_compare_callback = ptr;
    SREG = interrupt_state;
}

/** @copydoc TIMER0_OVF_interrupt_EN */
void TIMER0_OVF_interrupt_EN(void)
{
    SET_BIT(TIMSK, TOIE0);
}

/** @copydoc TIMER0_OCR0_interrupt_EN */
void TIMER0_OCR0_interrupt_EN(void)
{
    SET_BIT(TIMSK, OCIE0);
}

/** @copydoc TIMER0_OVF_interrupt_disable */
void TIMER0_OVF_interrupt_disable(void)
{
    CLEAR_BIT(TIMSK, TOIE0);
}

/** @copydoc TIMER0_OCR0_interrupt_disable */
void TIMER0_OCR0_interrupt_disable(void)
{
    CLEAR_BIT(TIMSK, OCIE0);
}

/** Invoke the registered overflow callback, when one is installed. */
ISR(TIMER0_OVF_vect)
{
    if (timer0_overflow_callback != 0) {
        timer0_overflow_callback();
    }
}

/** Invoke the registered output-compare callback, when one is installed. */
ISR(TIMER0_COMP_vect)
{
    if (timer0_compare_callback != 0) {
        timer0_compare_callback();
    }
}

/** @copydoc TIMER1_init */
void TIMER1_init(TIMER1_Mode_t mode,
                 TIMER1_OCR1A_mode_t oca,
                 TIMER1_OCR1B_mode_t ocb, 
                 TIMER1_CLK_t clk)
{
    /* Clear both control registers before applying the new configuration. */
    TCCR1A = 0;
    TCCR1B = 0;

    /* Configure waveform generation mode using WGM13:0. */
    switch (mode) {
        case TIMER1_NORMAL_MODE: 
            
            break;

        case TIMER1_PWM_PHASE_CORRECT_8BIT: 
            SET_BIT(TCCR1A, WGM10);
            break;

        case TIMER1_PWM_PHASE_CORRECT_9BIT: 
            SET_BIT(TCCR1A, WGM11);
            break;

        case TIMER1_PWM_PHASE_CORRECT_10BIT: 
            SET_BIT(TCCR1A, WGM10);
            SET_BIT(TCCR1A, WGM11);
            break;

        case TIMER1_CTC_OCR1A: 
            SET_BIT(TCCR1B, WGM12);
            break;

        case TIMER1_FAST_PWM_8BIT: 
            SET_BIT(TCCR1A, WGM10);
            SET_BIT(TCCR1B, WGM12);
            break;

        case TIMER1_FAST_PWM_9BIT: 
            SET_BIT(TCCR1A, WGM11);
            SET_BIT(TCCR1B, WGM12);
            break;

        case TIMER1_FAST_PWM_10BIT: 
            SET_BIT(TCCR1A, WGM10);
            SET_BIT(TCCR1A, WGM11);
            SET_BIT(TCCR1B, WGM12);
            break;

        case TIMER1_PWM_PHASE_FREQUENCY_CORRECT_ICR1: 
            SET_BIT(TCCR1B, WGM13);
            break;

        case TIMER1_PWM_PHASE_FREQUENCY_CORRECT_OCR1A: 
            SET_BIT(TCCR1A, WGM10);
            SET_BIT(TCCR1B, WGM13);
            break;

        case TIMER1_PWM_PHASE_CORRECT_ICR1: 
            SET_BIT(TCCR1A, WGM11);
            SET_BIT(TCCR1B, WGM13);
            break;

        case TIMER1_PWM_PHASE_CORRECT_OCR1A: 
            SET_BIT(TCCR1A, WGM10);
            SET_BIT(TCCR1A, WGM11);
            SET_BIT(TCCR1B, WGM13);
            break;

        case TIMER1_CTC_ICR1: 
            SET_BIT(TCCR1B, WGM12);
            SET_BIT(TCCR1B, WGM13);
            break;

        case TIMER1_FAST_PWM_ICR1: 
            SET_BIT(TCCR1A, WGM11);
            SET_BIT(TCCR1B, WGM12);
            SET_BIT(TCCR1B, WGM13);
            break;

        case TIMER1_FAST_PWM_OCR1A: 
            SET_BIT(TCCR1A, WGM10);
            SET_BIT(TCCR1A, WGM11);
            SET_BIT(TCCR1B, WGM12);
            SET_BIT(TCCR1B, WGM13);
            break;

        default:
            
            break;
    }

    /* Configure OC1A compare-output behavior using COM1A1:0. */
    switch (oca) {
        case TIMER1_OCR1A_NON_PWM_TOGGLE:
        case TIMER1_OCR1A_FAST_PWM_TOGGLE_ON_TOP:
        case TIMER1_OCR1A_PHASE_PWM_TOGGLE_ON_TOP:
            SET_BIT(TCCR1A, COM1A0);
            break;

        case TIMER1_OCR1A_NON_PWM_CLEAR:
        case TIMER1_OCR1A_FAST_PWM_NON_INVERTING:
        case TIMER1_OCR1A_PHASE_PWM_NON_INVERTING:
            SET_BIT(TCCR1A, COM1A1);
            break;

        case TIMER1_OCR1A_NON_PWM_SET: 
        case TIMER1_OCR1A_FAST_PWM_INVERTING: 
        case TIMER1_OCR1A_PHASE_PWM_INVERTING: 
            SET_BIT(TCCR1A, COM1A0);
            SET_BIT(TCCR1A, COM1A1);
            break;

        case TIMER1_OCR1A_DISCONNECTED:

            break;
    }

    /* Configure OC1B compare-output behavior using COM1B1:0. */
    switch (ocb) {
        case TIMER1_OCR1B_NON_PWM_TOGGLE:
            SET_BIT(TCCR1A, COM1B0);
            break;
        case TIMER1_OCR1B_NON_PWM_CLEAR:
        case TIMER1_OCR1B_FAST_PWM_NON_INVERTING:
        case TIMER1_OCR1B_PHASE_PWM_NON_INVERTING:
            SET_BIT(TCCR1A, COM1B1);
            break;
        case TIMER1_OCR1B_NON_PWM_SET:
        case TIMER1_OCR1B_FAST_PWM_INVERTING:
        case TIMER1_OCR1B_PHASE_PWM_INVERTING:
            SET_BIT(TCCR1A, COM1B0);
            SET_BIT(TCCR1A, COM1B1);
            break;
        case TIMER1_OCR1B_DISCONNECTED:
            break;
    }

    /* Select the Timer1 clock source using CS12:0. */
    switch (clk) {
        case TIMER1_NO_PRESCALER:
            SET_BIT(TCCR1B, CS10);
            break;

        case TIMER1_PRESCALER_8:
            SET_BIT(TCCR1B, CS11);
            break;

        case TIMER1_PRESCALER_64:
            SET_BIT(TCCR1B, CS10);
            SET_BIT(TCCR1B, CS11);
            break;

        case TIMER1_PRESCALER_256:
            SET_BIT(TCCR1B, CS12);
            break;

        case TIMER1_PRESCALER_1024:
            SET_BIT(TCCR1B, CS10);
            SET_BIT(TCCR1B, CS12);
            break;

        case TIMER1_EXTERNAL_FALLING_EDGE:
            SET_BIT(TCCR1B, CS11);
            SET_BIT(TCCR1B, CS12);
            break;

        case TIMER1_EXTERNAL_RISING_EDGE:
            SET_BIT(TCCR1B, CS10);
            SET_BIT(TCCR1B, CS11);
            SET_BIT(TCCR1B, CS12);
            break;

        case TIMER1_NO_CLK:
        default:
        
            break;
    }
}


/** @copydoc TIMER1_setOCR1A */
void TIMER1_setOCR1A(uint16_t value)
{
    OCR1A = value;
}

/** @copydoc TIMER1_setOCR1B */
void TIMER1_setOCR1B(uint16_t value)
{
    OCR1B = value;
}

/** @copydoc TIMER1_setICR1 */
void TIMER1_setICR1(uint16_t value)
{
    ICR1 = value;
}

/** @copydoc TIMER1_OVF_interrupt_init */
void TIMER1_OVF_interrupt_init(callback_t ptr)
{
    uint8_t interrupt_state = SREG;

    cli();
    timer1_overflow_callback = ptr;
    SREG = interrupt_state;
}

/** @copydoc TIMER1_OCR1A_interrupt_init */
void TIMER1_OCR1A_interrupt_init(callback_t ptr)
{
    uint8_t interrupt_state = SREG;

    cli();
    timer1_compare_a_callback = ptr;
    SREG = interrupt_state;
}

/** @copydoc TIMER1_OCR1B_interrupt_init */
void TIMER1_OCR1B_interrupt_init(callback_t ptr)
{
    uint8_t interrupt_state = SREG;

    cli();
    timer1_compare_b_callback = ptr;
    SREG = interrupt_state;
}

/** @copydoc TIMER1_OVF_interrupt_EN */
void TIMER1_OVF_interrupt_EN(void)
{
    SET_BIT(TIMSK, TOIE1);
}

/** @copydoc TIMER1_OCR1A_interrupt_EN */
void TIMER1_OCR1A_interrupt_EN(void)
{
    SET_BIT(TIMSK, OCIE1A);
}

/** @copydoc TIMER1_OCR1B_interrupt_EN */
void TIMER1_OCR1B_interrupt_EN(void)
{
    SET_BIT(TIMSK, OCIE1B);
}

/** @copydoc TIMER1_OVF_interrupt_disable */
void TIMER1_OVF_interrupt_disable(void)
{
    CLEAR_BIT(TIMSK, TOIE1);
}

/** @copydoc TIMER1_OCR1A_interrupt_disable */
void TIMER1_OCR1A_interrupt_disable(void)
{
    CLEAR_BIT(TIMSK, OCIE1A);
}

/** @copydoc TIMER1_OCR1B_interrupt_disable */
void TIMER1_OCR1B_interrupt_disable(void)
{
    CLEAR_BIT(TIMSK, OCIE1B);
}

/** Invoke the registered Timer1 overflow callback, when one is installed. */
ISR(TIMER1_OVF_vect)
{
    if (timer1_overflow_callback != 0) {
        timer1_overflow_callback();
    }
}
/** Invoke the registered compare-A callback, when one is installed. */
ISR(TIMER1_COMPA_vect)
{
    if (timer1_compare_a_callback != 0) {
        timer1_compare_a_callback();
    }
}

/** Invoke the registered compare-B callback, when one is installed. */
ISR(TIMER1_COMPB_vect)
{
    if (timer1_compare_b_callback != 0) {
        timer1_compare_b_callback();
    }
}

/* ---------------- Timer1 input capture (ICP1 / PD6) ---------------- */

/** @brief Input-capture ISR callback; null suppresses dispatch. */
static callback_t volatile timer1_capture_callback;

/** @copydoc TIMER1_ICP_setEdge */
void TIMER1_ICP_setEdge(TIMER1_ICP_edge_t edge)
{
    if (edge == TIMER1_ICP_RISING) {
        SET_BIT(TCCR1B, ICES1);
    } else {
        CLEAR_BIT(TCCR1B, ICES1);
    }
    /* Flags clear by writing one; do not use read-modify-write on TIFR. */
    TIFR = (1 << ICF1);
}

/** @copydoc TIMER1_getICR1 */
uint16_t TIMER1_getICR1(void)
{
    return ICR1;
}

/** @copydoc TIMER1_OVF_flag */
uint8_t TIMER1_OVF_flag(void)
{
    return (TIFR & (1 << TOV1)) ? 1 : 0;
}

/** @copydoc TIMER1_ICP_interrupt_init */
void TIMER1_ICP_interrupt_init(callback_t ptr)
{
    uint8_t interrupt_state = SREG;

    cli();
    timer1_capture_callback = ptr;
    SREG = interrupt_state;
}

/** @copydoc TIMER1_ICP_interrupt_EN */
void TIMER1_ICP_interrupt_EN(void)
{
    SET_BIT(TIMSK, TICIE1);
}

/** @copydoc TIMER1_ICP_interrupt_disable */
void TIMER1_ICP_interrupt_disable(void)
{
    CLEAR_BIT(TIMSK, TICIE1);
}

/** Invoke the registered input-capture callback, when one is installed. */
ISR(TIMER1_CAPT_vect)
{
    if (timer1_capture_callback != 0) {
        timer1_capture_callback();
    }
}
