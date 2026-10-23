/**
 * @file main.c
 * @author Mohamed
 * @date 2026-10-21
 * @brief PWM drawer: a mini oscilloscope on a 128x64 graphical LCD (ATmega32A, 16 MHz).
 *
 * Sources (selected by PD2, pull-up enabled):
 *   PD2 open / HIGH  -> EXTERNAL: signal on ICP1 (PD6), measured with Timer1 input capture.
 *   PD2 to GND / LOW -> INTERNAL: Timer0 fast PWM on OC0 (PB3), values taken from its settings.
 *
 * Internal source buttons (to GND, internal pull-ups):
 *   PD3 steps the frequency: 244 Hz -> 977 Hz -> 7.813 kHz -> 62.5 kHz
 *   PD4 steps the duty cycle: 10 -> 25 -> 50 -> 75 -> 90 %
 *
 * External measurement is gated: edges are accumulated for about 20 ms (at least one full cycle),
 * so frequency, duty and cycle time are averages over many cycles and do not flicker.
 *
 * Screen (128x64): frequency (upper left), duty (upper right), cycle time T with a marker over the
 * first period, 4 periods of the waveform, and the active source on the bottom line.
 * External range: roughly 1 Hz .. 50 kHz.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "CONFIG.h"
#include "MCAL/GPIO/GPIO.h"
#include "MCAL/DIO/DIO.h"
#include "MCAL/TIMER/TIMER.h"
#include "HAL/GLCD/GLCD.h"

/* ---------------------------- configuration ---------------------------- */
#define MODE_PIN          GPIO_PD2   /* LOW = internal, HIGH/open = external */
#define BTN_FREQ_PIN      GPIO_PD3   /* internal: next frequency preset       */
#define BTN_DUTY_PIN      GPIO_PD4   /* internal: next duty preset            */
#define ICP_PIN           GPIO_PD6   /* Timer1 input capture                   */
#define PWM_OUT_PIN       GPIO_PB3   /* OC0                                    */

#define TICK_HZ           (F_CPU / 8UL)   /* Timer1 at F_CPU/8 -> 0.5 us per tick */
#define GATE_TICKS        40000UL         /* 20 ms measurement window             */
#define MAX_CYCLES        2000U           /* keeps TICK_HZ*cycles within 32 bits  */

#define LOOP_DELAY_MS     100        /* also acts as button debounce            */
#define NO_SIGNAL_LOOPS   30         /* ~3 s without a full measurement -> NO SIGNAL */

/* waveform layout (absolute pixel coordinates) */
#define WAVE_X0           4
#define WAVE_CYCLE_PX     30
#define WAVE_CYCLES       4
#define WAVE_Y_HIGH       30         /* page 3 */
#define WAVE_Y_LOW        52         /* page 6 */
#define DIM_Y             20         /* dimension marker, page 2 */

/* ------------------------ internal source presets ----------------------- */
#define N_FREQ  4
#define N_DUTY  5
static const TIMER0_CLK_t freq_clk[N_FREQ] = {
    TIMER0_PRESCALER_256, TIMER0_PRESCALER_64, TIMER0_PRESCALER_8, TIMER0_NO_PRESCALER
};
static const uint16_t freq_div[N_FREQ]   = { 256, 64, 8, 1 };
static const uint8_t  duty_list[N_DUTY]  = { 10, 25, 50, 75, 90 };

static uint8_t freq_idx = 2;   /* 7.813 kHz */
static uint8_t duty_idx = 1;   /* 25 %      */
static uint8_t int_ocr0;

/** @brief Program Timer0 fast PWM from the selected presets. */
static void apply_internal(void)
{
    int_ocr0 = (uint8_t)(((uint16_t)duty_list[duty_idx] * 256 + 50) / 100 - 1);
    TIMER0_setOCR0(int_ocr0);
    TIMER0_init(TIMER0_FAST_PWM, TIMER0_FAST_PWM_OCR0_CLEAR_OC_SET_BOTTOM, freq_clk[freq_idx]);
}

/* ------------------------- capture state machine ------------------------ */
enum { CAP_WAIT_RISE1 = 0, CAP_WAIT_FALL, CAP_WAIT_RISE, CAP_DONE };

static volatile uint16_t ovf_count;      /* Timer1 overflows, extends the 16-bit timer to 32 */
static volatile uint8_t  cap_state;
static volatile uint32_t t_start, t_rise, cap_total, cap_high;
static volatile uint16_t cap_cycles;

static void on_overflow(void)
{
    ovf_count++;
}

/**
 * @brief ICP1 ISR callback. Starts at a rising edge, then accumulates high time and elapsed time
 *        over complete cycles until the gate time has passed, and stops itself.
 */
static void on_capture(void)
{
    uint16_t icr = TIMER1_getICR1();
    uint32_t ts  = ((uint32_t)ovf_count << 16) | icr;

    /* A capture just after an overflow whose ISR has not yet run belongs to the next epoch. */
    if (TIMER1_OVF_flag() && icr < 0x8000) ts += 0x10000UL;

    switch (cap_state) {
    case CAP_WAIT_RISE1:
        t_start = ts;
        t_rise  = ts;
        cap_high = 0;
        cap_cycles = 0;
        TIMER1_ICP_setEdge(TIMER1_ICP_FALLING);
        cap_state = CAP_WAIT_FALL;
        break;
    case CAP_WAIT_FALL:
        cap_high += ts - t_rise;
        TIMER1_ICP_setEdge(TIMER1_ICP_RISING);
        cap_state = CAP_WAIT_RISE;
        break;
    case CAP_WAIT_RISE:
        cap_cycles++;
        t_rise = ts;
        if ((ts - t_start) >= GATE_TICKS || cap_cycles >= MAX_CYCLES) {
            cap_total = ts - t_start;
            TIMER1_ICP_interrupt_disable();   /* hold the result until main consumes it */
            cap_state = CAP_DONE;
        } else {
            TIMER1_ICP_setEdge(TIMER1_ICP_FALLING);
            cap_state = CAP_WAIT_FALL;
        }
        break;
    default:
        break;
    }
}

static void capture_start(void)
{
    uint8_t sreg = SREG;
    cli();
    cap_state = CAP_WAIT_RISE1;
    TIMER1_ICP_setEdge(TIMER1_ICP_RISING);
    TIMER1_ICP_interrupt_EN();
    SREG = sreg;
}

static void capture_stop(void)
{
    TIMER1_ICP_interrupt_disable();
}

/* ------------------------------ scope data ------------------------------ */
typedef struct {
    uint8_t  external;
    uint8_t  valid;
    uint32_t hz;       /* frequency in Hz            */
    uint8_t  duty;     /* duty cycle in %            */
    uint32_t us10;     /* cycle time in 0.1 us units */
} scope_t;

static char txt_freq[16], txt_duty[8], txt_time[16], txt_src[16];
static uint8_t draw_wave_flag, wave_duty;

static void build_texts(const scope_t *s)
{
    snprintf(txt_src, sizeof txt_src, "SRC: %s", s->external ? "EXTERNAL" : "INTERNAL");

    if (!s->valid) {
        strcpy(txt_freq, "F=---");
        strcpy(txt_duty, "D=---");
        txt_time[0] = '\0';
        draw_wave_flag = 0;
        return;
    }

    snprintf(txt_freq, sizeof txt_freq, "F=%lu.%03lukHz",
             (unsigned long)(s->hz / 1000), (unsigned long)(s->hz % 1000));

    wave_duty = s->duty;
    snprintf(txt_duty, sizeof txt_duty, "D=%u%%", (unsigned)s->duty);

    if (s->us10 < 10000UL) {                         /* < 1 ms -> microseconds */
        snprintf(txt_time, sizeof txt_time, "T=%lu.%luus",
                 (unsigned long)(s->us10 / 10), (unsigned long)(s->us10 % 10));
    } else {                                         /* milliseconds, 2 decimals */
        uint32_t ms100 = s->us10 / 100;
        snprintf(txt_time, sizeof txt_time, "T=%lu.%02lums",
                 (unsigned long)(ms100 / 100), (unsigned long)(ms100 % 100));
    }
    draw_wave_flag = 1;
}

/* ------------------------------- rendering ------------------------------ */
static void draw_wave(uint8_t *b, uint8_t page, uint8_t duty)
{
    const uint8_t x_end = WAVE_X0 + WAVE_CYCLES * WAVE_CYCLE_PX;

    if (duty == 0)   { GLCD_BufHLine(b, page, WAVE_X0, x_end, WAVE_Y_LOW);  return; }
    if (duty >= 100) { GLCD_BufHLine(b, page, WAVE_X0, x_end, WAVE_Y_HIGH); return; }

    uint8_t hp = (uint8_t)(((uint16_t)duty * WAVE_CYCLE_PX + 50) / 100);
    if (hp < 1) hp = 1;
    if (hp > WAVE_CYCLE_PX - 1) hp = WAVE_CYCLE_PX - 1;

    for (uint8_t c = 0; c < WAVE_CYCLES; c++) {
        uint8_t x0 = (uint8_t)(WAVE_X0 + c * WAVE_CYCLE_PX);
        GLCD_BufVLine(b, page, x0,      WAVE_Y_HIGH, WAVE_Y_LOW);   /* rising edge  */
        GLCD_BufHLine(b, page, x0,      x0 + hp,           WAVE_Y_HIGH);
        GLCD_BufVLine(b, page, x0 + hp, WAVE_Y_HIGH, WAVE_Y_LOW);   /* falling edge */
        GLCD_BufHLine(b, page, x0 + hp, x0 + WAVE_CYCLE_PX, WAVE_Y_LOW);
    }
}

/** @brief Dimension marker "|<--->|" over the first period of the wave. */
static void draw_time_marker(uint8_t *b, uint8_t page)
{
    const uint8_t xa = WAVE_X0, xb = WAVE_X0 + WAVE_CYCLE_PX;
    GLCD_BufHLine(b, page, xa, xb, DIM_Y);
    GLCD_BufVLine(b, page, xa, DIM_Y - 3, DIM_Y + 3);
    GLCD_BufVLine(b, page, xb, DIM_Y - 3, DIM_Y + 3);
    GLCD_BufPixel(b, page, xa + 1, DIM_Y - 1);  GLCD_BufPixel(b, page, xa + 1, DIM_Y + 1);
    GLCD_BufPixel(b, page, xb - 1, DIM_Y - 1);  GLCD_BufPixel(b, page, xb - 1, DIM_Y + 1);
}

/** @brief Compose one 8-pixel-high page (text + graphics) and push it to the LCD. */
static void render_page(uint8_t page)
{
    uint8_t buf[GLCD_WIDTH];
    GLCD_BufClear(buf);

    if (page == 0) {
        GLCD_BufText(buf, txt_freq, 0);                                            /* upper left  */
        GLCD_BufText(buf, txt_duty, (uint8_t)(GLCD_WIDTH - 6 * strlen(txt_duty))); /* upper right */
    }
    if (page == 1 && txt_time[0]) GLCD_BufText(buf, txt_time, WAVE_X0);
    if (page == 4 && !draw_wave_flag) GLCD_BufText(buf, "NO SIGNAL", 37);
    if (page == 7) GLCD_BufText(buf, txt_src, 0);

    if (draw_wave_flag) {
        draw_time_marker(buf, page);
        draw_wave(buf, page, wave_duty);
    }
    GLCD_WritePage(page, buf);
}

static void render_screen(void)
{
    for (uint8_t p = 0; p < GLCD_PAGES; p++) render_page(p);
}

/* --------------------------------- main --------------------------------- */
int main(void)
{
    GLCD_Init();

    GPIO_Pin_Mode(MODE_PIN, GPIO_DIO_INPUT_PULLUP);
    GPIO_Pin_Mode(BTN_FREQ_PIN, GPIO_DIO_INPUT_PULLUP);
    GPIO_Pin_Mode(BTN_DUTY_PIN, GPIO_DIO_INPUT_PULLUP);
    GPIO_Pin_Mode(ICP_PIN, GPIO_DIO_INPUT_PULLUP);   /* idle-high: no edges -> NO SIGNAL */
    GPIO_Pin_Mode(PWM_OUT_PIN, GPIO_DIO_OUTPUT);

    apply_internal();   /* Timer0 fast PWM on OC0 */

    /* Measurement timebase: Timer1 free-running at F_CPU/8 with input capture on ICP1. */
    TIMER1_init(TIMER1_NORMAL_MODE, TIMER1_OCR1A_DISCONNECTED, TIMER1_OCR1B_DISCONNECTED,
                TIMER1_PRESCALER_8);
    TIMER1_OVF_interrupt_init(on_overflow);
    TIMER1_OVF_interrupt_EN();
    TIMER1_ICP_interrupt_init(on_capture);
    sei();

    scope_t cur = {0, 0, 0, 0, 0}, shown = {0xFF, 0xFF, 0, 0, 0};
    uint8_t prev_ext = 0xFF;
    uint8_t prev_bf = 0, prev_bd = 0;
    uint8_t idle = 0;

    for (;;) {
        /* internal-source buttons: act on a new press only */
        uint8_t bf = (DIO_read_pin(BTN_FREQ_PIN) == DIO_LOW);
        uint8_t bd = (DIO_read_pin(BTN_DUTY_PIN) == DIO_LOW);
        if (bf && !prev_bf) { freq_idx = (uint8_t)((freq_idx + 1) % N_FREQ); apply_internal(); }
        if (bd && !prev_bd) { duty_idx = (uint8_t)((duty_idx + 1) % N_DUTY); apply_internal(); }
        prev_bf = bf;
        prev_bd = bd;

        cur.external = (DIO_read_pin(MODE_PIN) == DIO_HIGH);

        if (!cur.external) {
            if (prev_ext != 0) capture_stop();
            uint16_t div = freq_div[freq_idx];
            cur.valid = 1;
            cur.hz    = (F_CPU + (256UL * div) / 2) / (256UL * div);
            cur.duty  = (uint8_t)((((uint16_t)int_ocr0 + 1) * 100 + 128) / 256);
            cur.us10  = 160UL * div;
        } else {
            if (prev_ext != 1) { capture_start(); idle = 0; cur.valid = 0; }

            if (cap_state == CAP_DONE) {
                uint32_t total  = cap_total;
                uint32_t high   = cap_high;
                uint16_t cycles = cap_cycles;
                if (total > 0 && cycles > 0 && high <= total) {
                    cur.valid = 1;
                    cur.hz    = (TICK_HZ * cycles + total / 2) / total;
                    cur.duty  = (uint8_t)((high * 100UL + total / 2) / total);
                    cur.us10  = (total * 5UL + cycles / 2) / cycles;
                } else {
                    cur.valid = 0;
                }
                capture_start();                 /* arm the next measurement */
                idle = 0;
            } else if (idle < NO_SIGNAL_LOOPS) {
                idle++;
            } else {                             /* no complete measurement for ~3 s */
                cur.valid = 0;
                capture_start();
                idle = 0;
            }
        }
        prev_ext = cur.external;

        if (cur.external != shown.external || cur.valid != shown.valid ||
            (cur.valid && (cur.hz != shown.hz || cur.duty != shown.duty ||
                           cur.us10 != shown.us10))) {
            shown = cur;
            build_texts(&cur);
            render_screen();
        }
        _delay_ms(LOOP_DELAY_MS);
    }
}
