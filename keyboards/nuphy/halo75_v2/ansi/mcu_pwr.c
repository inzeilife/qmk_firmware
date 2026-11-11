/*
Copyright 2023 NuPhy, Persama (@Persama) & jincao1

Adapted from ryodeushii/qmk-firmware
(keyboards/nuphy/common/power/mcu_pwr.c) for the upstream QMK port.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ansi.h"
#include "mcu_pwr.h"
#include "rf_driver.h"
#include "hal.h"
#include "hal_lld.h"
#include "hal_usb.h"
#include "usb_main.h"
#include "rgb_matrix.h"

extern DEV_INFO_STRUCT dev_info;
extern bool            f_wakeup_prepare;
extern bool            f_goto_sleep;
extern uint16_t        no_act_time;
extern uint16_t        rf_linking_time;

extern uint8_t uart_send_cmd(uint8_t cmd, uint8_t ack_cnt, uint8_t delayms);
extern void    m_break_all_key(void);
extern void    matrix_init_custom(void);
extern void    matrix_capture_wake(void);

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

static bool sleeping            = false;
static bool rgb_led_on          = true;
static bool rgb_led_powered_off = false;

static void rf_sleep(void) {
    if (dev_info.rf_state == RF_CONNECT) {
        uart_send_cmd(CMD_SET_CONFIG, 5, 5);
    } else {
        uart_send_cmd(CMD_SLEEP, 5, 5);
    }
}

static void rf_wake(void) {
    if (dev_info.link_mode != LINK_USB) {
        dev_info.rf_state = RF_IDLE;
    }
    uart_send_cmd(CMD_HAND, 0, 1);
    rf_start_wakeup_replay();
}

void pwr_rgb_led_off(void) {
    if (!rgb_led_on) return;

    rgb_matrix_set_color_all(0, 0, 0);
    rgb_matrix_update_pwm_buffers();

    writePinLow(DC_BOOST_PIN);
    writePinLow(RGB_DRIVER_SDB1);
    writePinLow(RGB_DRIVER_SDB2);

    rgb_led_on = false;
}

void pwr_rgb_led_on(void) {
    if (sleeping || rgb_led_on) return;

    writePinHigh(DC_BOOST_PIN);
    writePinHigh(RGB_DRIVER_SDB1);
    writePinHigh(RGB_DRIVER_SDB2);

    rgb_led_on = true;
}

bool is_rgb_led_on(void) {
    return rgb_led_on;
}

bool pwr_is_sleeping(void) {
    return sleeping;
}

void led_pwr_sleep_handle(void) {
    if (rgb_led_on) {
        rgb_led_powered_off = true;
        pwr_rgb_led_off();
    }
}

void led_pwr_wake_handle(void) {
    if (!rgb_led_powered_off) return;

    rgb_matrix_set_color_all(0, 0, 0);
    pwr_rgb_led_on();
    rgb_matrix_driver.init();
    rgb_matrix_update_pwm_buffers();

    rgb_led_powered_off = false;
}

void enter_light_sleep(void) {
    if (sleeping) return;

    rf_cancel_wakeup_replay();
    rf_sleep();
    rf_start_wakeup_replay();
    led_pwr_sleep_handle();

    sleeping = true;
}

void exit_light_sleep(void) {
    if (!sleeping) return;

    sleeping = false;

    led_pwr_wake_handle();
    rf_wake();

    if (dev_info.link_mode == LINK_USB) {
#define USB_GETSTATUS_REMOTE_WAKEUP_ENABLED (2U)
        if ((USB_DRIVER.status & USB_GETSTATUS_REMOTE_WAKEUP_ENABLED)) {
            usb_lld_wakeup_host(&USB_DRIVER);
            wait_ms(50);

            uint8_t timeout = 10;
            while ((USB_DRIVER.state == USB_SUSPENDED) && (timeout--)) {
                usbWakeupHost(&USB_DRIVER);
                restart_usb_driver(&USB_DRIVER);
                wait_ms(50);
            }

            m_break_all_key();
        }
    }
}

void enter_deep_sleep(void) {
    if (!sleeping) {
        m_break_all_key();
        rf_sleep();
        led_pwr_sleep_handle();
    }

    rf_cancel_wakeup_replay();
    rf_start_wakeup_replay();

    for (uint8_t i = 0; i < MATRIX_COLS; i++) {
        gpio_set_pin_output(col_pins[i]);
        gpio_write_pin_high(col_pins[i]);
    }

    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        gpio_set_pin_input_low(row_pins[i]);
        palEnableLineEvent(row_pins[i], PAL_EVENT_MODE_BOTH_EDGES);
    }

    gpio_set_pin_output(DEV_MODE_PIN);
    gpio_write_pin_low(DEV_MODE_PIN);

    gpio_set_pin_output(SYS_MODE_PIN);
    gpio_write_pin_low(SYS_MODE_PIN);

    gpio_set_pin_input(NRF_WAKEUP_PIN);
    gpio_write_pin_low(NRF_WAKEUP_PIN);

    sleeping = true;

    uint32_t systick_ctrl = SysTick->CTRL;
    SysTick->CTRL = systick_ctrl & ~SysTick_CTRL_TICKINT_Msk;
    SCB->ICSR     = SCB_ICSR_PENDSTCLR_Msk;

    PWR->CR |= PWR_CR_CWUF | PWR_CR_CSBF;
    PWR->CR &= ~PWR_CR_PDDS;
    PWR->CR |= PWR_CR_LPDS;

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

    SysTick->CTRL = systick_ctrl;
}

void exit_deep_sleep(void) {
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        palDisableLineEvent(row_pins[i]);
    }

    stm32_clock_init();
    matrix_init_custom();
    matrix_capture_wake();

    gpio_set_pin_input_high(DEV_MODE_PIN);
    gpio_set_pin_input_high(SYS_MODE_PIN);

    gpio_set_pin_output(NRF_WAKEUP_PIN);
    gpio_write_pin_high(NRF_WAKEUP_PIN);

    sleeping = false;

    led_pwr_wake_handle();
    rf_wake();

    no_act_time      = 0;
    rf_linking_time  = 0;
    f_wakeup_prepare = 0;
    f_goto_sleep     = 0;

    if (dev_info.link_mode == LINK_USB && USB_DRIVER.state == USB_SUSPENDED) {
        usb_lld_wakeup_host(&USB_DRIVER);
    }
}
