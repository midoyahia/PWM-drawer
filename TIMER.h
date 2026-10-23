/**
 * @file TIMER.h
 * @brief Timer0 and Timer1 driver interfaces for the AVR ATmega32A timer peripherals.
 *
 * Provides configuration types for Timer0 waveform generation, OC0 compare
 * output behavior, clock source selection, compare value loading, and Timer0
 * interrupt callback registration. Also provides Timer1 configuration types
 * and prototypes for WGM13:0 modes, OC1A/OC1B compare output behavior,
 * Timer1 top/compare register loading, and Timer1 interrupt control.
 * @note Initialization rewrites control registers but does not reset counters,
 * compare/TOP registers, interrupt masks, pending flags, or callbacks.
 * Configure the output pin direction separately when using OC outputs.
 * @warning Callbacks run in ISR context and must be short and nonblocking.
 * Peripheral interrupt enables do not enable global interrupts.
 * @details With internal clock prescaler N, timer ticks occur at F_CPU / N.
 * In CTC with TOP in OCR, compare-event rate is F_CPU / (N * (OCR + 1));
 * toggling once per event gives half that frequency. Fast PWM repetition
 * is F_CPU / (N * (TOP + 1)); dual-slope PWM is F_CPU / (2 * N * TOP).
 * These equations describe steady operation with valid mode/TOP settings;
 * output toggle modes can differ from the PWM repetition frequency.
 */
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/** @brief Selects the Timer0 waveform generation mode. */
typedef enum {
    TIMER0_NORMAL_MODE,
    TIMER0_CTC_MODE,
    TIMER0_FAST_PWM,
    TIMER0_PHASECORRECT_PWM
} TIMER0_Mode_t;

/** @brief Selects how the OC0 pin behaves on compare match. */
typedef enum {
    TIMER0_OCR0_DISCONNECTED,
    TIMER0_NON_PWM_OCR0_TOGGLE,
    TIMER0_NON_PWM_OCR0_SET,
    TIMER0_NON_PWM_OCR0_CLEAR,
    TIMER0_FAST_PWM_OCR0_CLEAR_OC_SET_BOTTOM,
    TIMER0_FAST_PWM_OCR0_SET_OC_CLEAR_BOTTOM,
    TIMER0_PHASE_PWM_OCR0_CLEAR_UP_SET_DOWN,
    TIMER0_PHASE_PWM_OCR0_SET_UP_CLEAR_DOWN
} TIMER0_compare_output_mode_t;

/** @brief Selects the Timer0 clock source and prescaler. */
typedef enum {
    TIMER0_NO_CLK,
    TIMER0_NO_PRESCALER,
    TIMER0_PRESCALER_8,
    TIMER0_PRESCALER_64,
    TIMER0_PRESCALER_256,
    TIMER0_PRESCALER_1024,
    TIMER0_EXTERNAL_FALLING_EDGE,
    TIMER0_EXTERNAL_RISING_EDGE
} TIMER0_CLK_t;

/** @brief Function pointer type used for Timer0 and Timer1 interrupt callbacks. */
typedef void (*callback_t)(void);

/**
 * @brief Configure Timer0 waveform, compare output, and clock source.
 * @param mode Normal, CTC, fast PWM, or phase-correct PWM.
 * @param cmpr Compare output action appropriate to the selected waveform mode.
 * @param clk Clock source/divider, or TIMER0_NO_CLK to stop counting.
 * @details Clears TCCR0 before installing new fields; a running clock starts
 * counting as soon as its clock-select bits are set.
 * @note Does not validate combinations, initialize OCR0/TCNT0, or configure OC0 DDR.
 */
void TIMER0_init(TIMER0_Mode_t mode,
                 TIMER0_compare_output_mode_t cmpr,
                 TIMER0_CLK_t clk);

/**
 * @brief Write the 8-bit OCR0 register.
 * @param value Raw register value, not a frequency or duration.
 * @details Writes the supplied value directly without validation or waiting.
 * @note In CTC this sets TOP. Lowering TOP below TCNT0 can miss the next
 * match until the counter wraps; the setter does not reset TCNT0.
 */
void TIMER0_setOCR0(uint8_t value);

/**
 * @brief Register the OVF interrupt callback.
 * @param ptr Function called from the ISR, or null to skip callback dispatch.
 * @details Saves SREG, disables interrupts while updating the pointer, then
 * restores SREG so the previous global interrupt state is preserved.
 * @note Registration does not enable this interrupt source.
 */
void TIMER0_OVF_interrupt_init(callback_t ptr);

/**
 * @brief Register the OCR0 interrupt callback.
 * @param ptr Function called from the ISR, or null to skip callback dispatch.
 * @details Saves SREG, disables interrupts while updating the pointer, then
 * restores SREG so the previous global interrupt state is preserved.
 * @note Registration does not enable this interrupt source.
 */
void TIMER0_OCR0_interrupt_init(callback_t ptr);

/**
 * @brief Enable the OVF interrupt source for Timer0.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER0_OVF_interrupt_EN(void);

/**
 * @brief Enable the OCR0 interrupt source for Timer0.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER0_OCR0_interrupt_EN(void);

/**
 * @brief Disable the OVF interrupt source for Timer0.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER0_OVF_interrupt_disable(void);

/**
 * @brief Disable the OCR0 interrupt source for Timer0.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER0_OCR0_interrupt_disable(void);




/**
 * @brief Selects a Timer1 waveform generation mode.
 *
 * Enumerator values match the ATmega32A WGM13:0 bit encoding split between
 * TCCR1A and TCCR1B. Mode 13 is reserved by the device and is intentionally
 * not exposed.
 */
typedef enum {
    TIMER1_NORMAL_MODE                         = 0,
    TIMER1_PWM_PHASE_CORRECT_8BIT              = 1,
    TIMER1_PWM_PHASE_CORRECT_9BIT              = 2,
    TIMER1_PWM_PHASE_CORRECT_10BIT             = 3,
    TIMER1_CTC_OCR1A                            = 4,
    TIMER1_FAST_PWM_8BIT                       = 5,
    TIMER1_FAST_PWM_9BIT                       = 6,
    TIMER1_FAST_PWM_10BIT                      = 7,
    TIMER1_PWM_PHASE_FREQUENCY_CORRECT_ICR1    = 8,
    TIMER1_PWM_PHASE_FREQUENCY_CORRECT_OCR1A   = 9,
    TIMER1_PWM_PHASE_CORRECT_ICR1              = 10,
    TIMER1_PWM_PHASE_CORRECT_OCR1A             = 11,
    TIMER1_CTC_ICR1                             = 12,
    /* WGM mode 13 is reserved. */
    TIMER1_FAST_PWM_ICR1                       = 14,
    TIMER1_FAST_PWM_OCR1A                      = 15
} TIMER1_Mode_t;

/**
 * @brief Selects the OC1A pin behavior on Timer1 compare match.
 *
 * The driver translates each option to COM1A1:0. Separate names are provided
 * because the same register encoding has different meanings in non-PWM,
 * Fast PWM, and phase-correct PWM modes.
 */
typedef enum {
    TIMER1_OCR1A_DISCONNECTED                    ,

    /* Normal and CTC modes */
    TIMER1_OCR1A_NON_PWM_TOGGLE                  ,
    TIMER1_OCR1A_NON_PWM_CLEAR                   ,
    TIMER1_OCR1A_NON_PWM_SET                     ,

    /* Fast PWM modes */
    TIMER1_OCR1A_FAST_PWM_TOGGLE_ON_TOP          ,
    TIMER1_OCR1A_FAST_PWM_NON_INVERTING          ,
    TIMER1_OCR1A_FAST_PWM_INVERTING              ,

    /* Phase-correct and phase/frequency-correct PWM modes */
    TIMER1_OCR1A_PHASE_PWM_TOGGLE_ON_TOP         ,
    TIMER1_OCR1A_PHASE_PWM_NON_INVERTING         ,
    TIMER1_OCR1A_PHASE_PWM_INVERTING             
} TIMER1_OCR1A_mode_t;


/**
 * @brief Selects the OC1B pin behavior on Timer1 compare match.
 *
 * The driver translates each option to COM1B1:0. OC1B does not support the
 * OCR1A-only toggle-on-TOP behavior used by some Timer1 modes.
 */
typedef enum {
    TIMER1_OCR1B_DISCONNECTED           ,

    /* Normal and CTC modes */
    TIMER1_OCR1B_NON_PWM_TOGGLE         ,
    TIMER1_OCR1B_NON_PWM_CLEAR          ,
    TIMER1_OCR1B_NON_PWM_SET            ,

    /* Fast PWM modes */
    TIMER1_OCR1B_FAST_PWM_NON_INVERTING ,
    TIMER1_OCR1B_FAST_PWM_INVERTING     ,

    /* Phase-correct and phase/frequency-correct PWM modes */
    TIMER1_OCR1B_PHASE_PWM_NON_INVERTING,
    TIMER1_OCR1B_PHASE_PWM_INVERTING    
} TIMER1_OCR1B_mode_t;

/** @brief Selects the Timer1 clock source and prescaler. */
typedef enum {
    TIMER1_NO_CLK,
    TIMER1_NO_PRESCALER,
    TIMER1_PRESCALER_8,
    TIMER1_PRESCALER_64,
    TIMER1_PRESCALER_256,
    TIMER1_PRESCALER_1024,
    TIMER1_EXTERNAL_FALLING_EDGE,
    TIMER1_EXTERNAL_RISING_EDGE
} TIMER1_CLK_t;

/**
 * @brief Configures Timer1 waveform mode, OC1A/OC1B output modes, and clock source.
 *
 * @param mode Timer1 waveform generation mode using the WGM13:0 encoding.
 * @param oca OC1A compare output behavior using COM1A1:0 encoding.
 * @param ocb OC1B compare output behavior using COM1B1:0 encoding.
 * @param clk Timer1 clock source or prescaler selection using CS12:0 encoding.
 * @details Clears TCCR1A/B before installing the new configuration.
 * @note Callers must choose compatible output modes and initialize TOP/compare
 * values and output directions. No combination validation is performed.
 */
void TIMER1_init(TIMER1_Mode_t mode,
                 TIMER1_OCR1A_mode_t oca,
                 TIMER1_OCR1B_mode_t ocb, 
                 TIMER1_CLK_t clk);

/**
 * @brief Write the 16-bit OCR1A register.
 * @param value Raw register value, not a frequency or duration.
 * @details Writes the supplied value directly without validation or waiting.
 * @note Acts as TOP in modes selecting OCR1A as TOP; otherwise a compare value.
 * @warning This setter does not guard the multi-byte access against interrupts;
 * coordinate with ISRs accessing Timer1 16-bit registers.
 */
void TIMER1_setOCR1A(uint16_t value);

/**
 * @brief Write the 16-bit OCR1B register.
 * @param value Raw register value, not a frequency or duration.
 * @details Writes the supplied value directly without validation or waiting.
 * @warning This setter does not guard the multi-byte access against interrupts;
 * coordinate with ISRs accessing Timer1 16-bit registers.
 */
void TIMER1_setOCR1B(uint16_t value);

/**
 * @brief Write the 16-bit ICR1 register.
 * @param value Raw register value, not a frequency or duration.
 * @details Writes the supplied value directly without validation or waiting.
 * @note Acts as TOP only in modes selecting ICR1 as TOP.
 * @warning This setter does not guard the multi-byte access against interrupts;
 * coordinate with ISRs accessing Timer1 16-bit registers.
 */
void TIMER1_setICR1(uint16_t value);

/**
 * @brief Register the OVF interrupt callback.
 * @param ptr Function called from the ISR, or null to skip callback dispatch.
 * @details Saves SREG, disables interrupts while updating the pointer, then
 * restores SREG so the previous global interrupt state is preserved.
 * @note Registration does not enable this interrupt source.
 */
void TIMER1_OVF_interrupt_init(callback_t ptr);

/**
 * @brief Register the OCR1A interrupt callback.
 * @param ptr Function called from the ISR, or null to skip callback dispatch.
 * @details Saves SREG, disables interrupts while updating the pointer, then
 * restores SREG so the previous global interrupt state is preserved.
 * @note Registration does not enable this interrupt source.
 */
void TIMER1_OCR1A_interrupt_init(callback_t ptr);

/**
 * @brief Register the OCR1B interrupt callback.
 * @param ptr Function called from the ISR, or null to skip callback dispatch.
 * @details Saves SREG, disables interrupts while updating the pointer, then
 * restores SREG so the previous global interrupt state is preserved.
 * @note Registration does not enable this interrupt source.
 */
void TIMER1_OCR1B_interrupt_init(callback_t ptr);

/**
 * @brief Enable the OVF interrupt source for Timer1.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER1_OVF_interrupt_EN(void);

/**
 * @brief Enable the OCR1A interrupt source for Timer1.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER1_OCR1A_interrupt_EN(void);

/**
 * @brief Enable the OCR1B interrupt source for Timer1.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER1_OCR1B_interrupt_EN(void);

/**
 * @brief Disable the OVF interrupt source for Timer1.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER1_OVF_interrupt_disable(void);

/**
 * @brief Disable the OCR1A interrupt source for Timer1.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER1_OCR1A_interrupt_disable(void);

/**
 * @brief Disable the OCR1B interrupt source for Timer1.
 * @details Changes only the corresponding TIMSK mask bit.
 * @note Does not stop the timer, reset its counter, clear pending flags,
 * or change the global interrupt state.
 */
void TIMER1_OCR1B_interrupt_disable(void);


/** @brief Selects which Timer1 input-capture (ICP1, PD6) edge latches ICR1. */
typedef enum {
    TIMER1_ICP_FALLING,
    TIMER1_ICP_RISING
} TIMER1_ICP_edge_t;

/**
 * @brief Select the active input-capture edge and clear the capture flag.
 * @param edge Rising or falling edge on ICP1.
 * @note Changing ICES1 can raise ICF1 spuriously; the flag is cleared here.
 */
void TIMER1_ICP_setEdge(TIMER1_ICP_edge_t edge);

/** @brief Read the 16-bit input-capture register ICR1 (call from the capture callback). */
uint16_t TIMER1_getICR1(void);

/** @brief Return nonzero while the Timer1 overflow flag (TOV1) is pending. */
uint8_t TIMER1_OVF_flag(void);

/**
 * @brief Register the input-capture interrupt callback.
 * @param ptr Function called from the ISR, or null to skip callback dispatch.
 * @note Registration does not enable the interrupt source.
 */
void TIMER1_ICP_interrupt_init(callback_t ptr);

/** @brief Enable the Timer1 input-capture interrupt source (TICIE1). */
void TIMER1_ICP_interrupt_EN(void);

/** @brief Disable the Timer1 input-capture interrupt source (TICIE1). */
void TIMER1_ICP_interrupt_disable(void);


#endif /* TIMER_H */
