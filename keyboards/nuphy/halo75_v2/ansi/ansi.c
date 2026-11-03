/*
Copyright 2023 @ Nuphy <https://nuphy.com/>

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
#include "is31fl3733.h"
#include "usb_main.h"
#include "rf_driver.h"


user_config_t user_config;  
DEV_INFO_STRUCT dev_info =
    {
        .rf_baterry = 100,
        .link_mode  = LINK_USB,
        .rf_state   = RF_IDLE,
};

uint16_t rf_linking_time            = 0;   
uint16_t rf_link_show_time          = 0; 
uint8_t rf_blink_cnt                = 0;     
uint16_t no_act_time                = 0;  
host_driver_t *m_host_driver        = 0;
uint16_t dev_reset_press_delay      = 0; 
uint16_t rf_sw_press_delay          = 0; 
uint16_t rgb_test_press_delay       = 0; 
uint8_t rf_sw_temp                  = 0;
uint8_t rgb_light_old               = 0;
uint8_t host_mode;

extern uint8_t side_mode_a;  
extern uint8_t side_light; 
extern uint8_t side_speed; 
extern uint8_t side_rgb;   
extern uint8_t side_colour;
extern report_keyboard_t *keyboard_report;
extern report_nkro_t *nkro_report;
extern uint8_t side_mode_b;  
extern uint8_t uart_bit_report_buf[32];
extern uint8_t bitkb_report_buf[32];
extern uint8_t bytekb_report_buf[8]; 

bool f_uart_ack         = 0; 
bool f_bat_show         = 0; 
bool f_bat_hold         = 0;
bool f_chg_show         = 1;
bool f_sys_show         = 0;
bool f_sleep_show       = 0; 
bool f_usb_offline      = 0; 
bool f_rf_read_data_ok  = 0;
bool f_rf_sts_sysc_ok   = 0;
bool f_rf_new_adv_ok    = 0;
bool f_rf_reset         = 0;
bool f_send_channel     = 0; 
bool f_rf_hand_ok       = 0;  
bool f_rf_send_bitkb    = 0; 
bool f_rf_send_byte     = 0; 
bool f_rf_send_consume  = 0; 
bool f_wakeup_prepare   = 0;
bool f_dial_sw_init_ok  = 0; 
bool f_goto_sleep       = 0; 
bool f_rf_sw_press      = 0; 
bool f_dev_reset_press  = 0; 
bool f_rgb_test_press   = 0; 
bool f_win_lock         = 0; 

void rf_device_init(void);
void rf_uart_init(void);
void m_side_led_show(void);
void dev_sts_sync(void);
void uart_receive_pro(void);
void Sleep_Handle(void);
void uart_send_report_func(void);
uint8_t uart_send_cmd(uint8_t cmd, uint8_t ack_cnt, uint8_t delayms);
void uart_send_report(uint8_t report_type, uint8_t *report_buf, uint8_t report_size);
void device_reset_show(void);
void device_reset_init(void);
void rgb_test_show(void);
void m_deinit_usb_072(void);

extern void light_speed_control(uint8_t fast);
extern void light_level_control(uint8_t brighten);
extern void side_colour_control(uint8_t dir);
extern void side_mode_a_control(uint8_t dir);
extern void side_mode_b_control(uint8_t dir);
extern bool low_bat_flag;


const is31fl3733_led_t PROGMEM g_is31fl3733_leds[IS31FL3733_LED_COUNT] = {
    // Row 1
    {0, SW1_CS6, SW2_CS6, SW3_CS6},    // {0, A_6, B_6, C_6}
    {0, SW1_CS7, SW2_CS7, SW3_CS7},    // {0, A_7, B_7, C_7}
    {0, SW1_CS8, SW2_CS8, SW3_CS8},    // {0, A_8, B_8, C_8}
    {0, SW1_CS9, SW2_CS9, SW3_CS9},    // {0, A_9, B_9, C_9}
    {0, SW1_CS10, SW2_CS10, SW3_CS10}, // {0, A_10, B_10, C_10}
    {0, SW1_CS11, SW2_CS11, SW3_CS11}, // {0, A_11, B_11, C_11}
    {0, SW1_CS12, SW2_CS12, SW3_CS12}, // {0, A_12, B_12, C_12}
    {1, SW1_CS1, SW2_CS1, SW3_CS1},    // {1, A_1, B_1, C_1}
    {1, SW1_CS2, SW2_CS2, SW3_CS2},    // {1, A_2, B_2, C_2}
    {1, SW1_CS3, SW2_CS3, SW3_CS3},    // {1, A_3, B_3, C_3}
    {1, SW1_CS4, SW2_CS4, SW3_CS4},    // {1, A_4, B_4, C_4}
    {1, SW1_CS5, SW2_CS5, SW3_CS5},    // {1, A_5, B_5, C_5}
    {1, SW1_CS6, SW2_CS6, SW3_CS6},    // {1, A_6, B_6, C_6}
    {1, SW1_CS7, SW2_CS7, SW3_CS7},    // {1, A_7, B_7, C_7}
    {1, SW1_CS9, SW2_CS9, SW3_CS9},    // {1, A_9, B_9, C_9}
    {1, SW1_CS8, SW2_CS8, SW3_CS8},    // {1, A_8, B_8, C_8}

    // Row 2
    {0, SW4_CS1, SW5_CS1, SW6_CS1},    // {0, D_1, E_1, F_1}
    {0, SW4_CS2, SW5_CS2, SW6_CS2},    // {0, D_2, E_2, F_2}
    {0, SW4_CS3, SW5_CS3, SW6_CS3},    // {0, D_3, E_3, F_3}
    {0, SW4_CS4, SW5_CS4, SW6_CS4},    // {0, D_4, E_4, F_4}
    {0, SW1_CS13, SW2_CS13, SW3_CS13}, // {0, A_13, B_13, C_13}
    {0, SW1_CS14, SW2_CS14, SW3_CS14}, // {0, A_14, B_14, C_14}
    {0, SW1_CS15, SW2_CS15, SW3_CS15}, // {0, A_15, B_15, C_15}
    {0, SW1_CS16, SW2_CS16, SW3_CS16}, // {0, A_16, B_16, C_16}
    {1, SW1_CS10, SW2_CS10, SW3_CS10}, // {1, A_10, B_10, C_10}
    {1, SW1_CS11, SW2_CS11, SW3_CS11}, // {1, A_11, B_11, C_11}
    {1, SW1_CS12, SW2_CS12, SW3_CS12}, // {1, A_12, B_12, C_12}
    {1, SW1_CS13, SW2_CS13, SW3_CS13}, // {1, A_13, B_13, C_13}
    {1, SW1_CS14, SW2_CS14, SW3_CS14}, // {1, A_14, B_14, C_14}
    {1, SW1_CS15, SW2_CS15, SW3_CS15}, // {1, A_15, B_15, C_15}
    {1, SW1_CS16, SW2_CS16, SW3_CS16}, // {1, A_16, B_16, C_16}

    // Row 3
    {0, SW4_CS5, SW5_CS5, SW6_CS5},    // {0, D_5, E_5, F_5}
    {0, SW4_CS6, SW5_CS6, SW6_CS6},    // {0, D_6, E_6, F_6}
    {0, SW4_CS7, SW5_CS7, SW6_CS7},    // {0, D_7, E_7, F_7}
    {0, SW4_CS8, SW5_CS8, SW6_CS8},    // {0, D_8, E_8, F_8}
    {0, SW4_CS9, SW5_CS9, SW6_CS9},    // {0, D_9, E_9, F_9}
    {0, SW4_CS10, SW5_CS10, SW6_CS10}, // {0, D_10, E_10, F_10}
    {0, SW4_CS11, SW5_CS11, SW6_CS11}, // {0, D_11, E_11, F_11}
    {1, SW4_CS1, SW5_CS1, SW6_CS1},    // {1, D_1, E_1, F_1}
    {1, SW4_CS2, SW5_CS2, SW6_CS2},    // {1, D_2, E_2, F_2}
    {1, SW4_CS3, SW5_CS3, SW6_CS3},    // {1, D_3, E_3, F_3}
    {1, SW4_CS4, SW5_CS4, SW6_CS4},    // {1, D_4, E_4, F_4}
    {1, SW4_CS5, SW5_CS5, SW6_CS5},    // {1, D_5, E_5, F_5}
    {1, SW4_CS6, SW5_CS6, SW6_CS6},    // {1, D_6, E_6, F_6}
    {1, SW4_CS7, SW5_CS7, SW6_CS7},    // {1, D_7, E_7, F_7}
    {1, SW4_CS8, SW5_CS8, SW6_CS8},    // {1, D_8, E_8, F_8}

    // Row 4
    {0, SW7_CS1, SW8_CS1, SW9_CS1}, // {0, G_1, H_1, I_1}
    {0, SW7_CS2, SW8_CS2, SW9_CS2}, // {0, G_2, H_2, I_2}
    {0, SW7_CS3, SW8_CS3, SW9_CS3}, // {0, G_3, H_3, I_3}
    {0, SW7_CS4, SW8_CS4, SW9_CS4}, // {0, G_4, H_4, I_4}
    {0, SW7_CS5, SW8_CS5, SW9_CS5}, // {0, G_5, H_5, I_5}
    {0, SW7_CS6, SW8_CS6, SW9_CS6}, // {0, G_6, H_6, I_6}
    {0, SW7_CS7, SW8_CS7, SW9_CS7}, // {0, G_7, H_7, I_7}
    {0, SW7_CS8, SW8_CS8, SW9_CS8}, // {0, G_8, H_8, I_8}
    {1, SW7_CS1, SW8_CS1, SW9_CS1}, // {1, G_1, H_1, I_1}
    {1, SW7_CS2, SW8_CS2, SW9_CS2}, // {1, G_2, H_2, I_2}
    {1, SW7_CS3, SW8_CS3, SW9_CS3}, // {1, G_3, H_3, I_3}
    {1, SW7_CS4, SW8_CS4, SW9_CS4}, // {1, G_4, H_4, I_4}
    {1, SW7_CS5, SW8_CS5, SW9_CS5}, // {1, G_5, H_5, I_5}
    {1, SW7_CS6, SW8_CS6, SW9_CS6}, // {1, G_6, H_6, I_6}

    // Row 5
    {0, SW10_CS1, SW11_CS1, SW12_CS1}, // {0, J_1, K_1, L_1}
    {0, SW10_CS2, SW11_CS2, SW12_CS2}, // {0, J_2, K_2, L_2}
    {0, SW10_CS3, SW11_CS3, SW12_CS3}, // {0, J_3, K_3, L_3}
    {0, SW10_CS4, SW11_CS4, SW12_CS4}, // {0, J_4, K_4, L_4}
    {0, SW10_CS5, SW11_CS5, SW12_CS5}, // {0, J_5, K_5, L_5}
    {0, SW10_CS6, SW11_CS6, SW12_CS6}, // {0, J_6, K_6, L_6}
    {0, SW10_CS7, SW11_CS7, SW12_CS7}, // {0, J_7, K_7, L_7}
    {0, SW10_CS8, SW11_CS8, SW12_CS8}, // {0, J_8, K_8, L_8}
    {1, SW10_CS1, SW11_CS1, SW12_CS1}, // {1, J_1, K_1, L_1}
    {1, SW10_CS2, SW11_CS2, SW12_CS2}, // {1, J_2, K_2, L_2}
    {1, SW10_CS3, SW11_CS3, SW12_CS3}, // {1, J_3, K_3, L_3}
    {1, SW10_CS4, SW11_CS4, SW12_CS4}, // {1, J_4, K_4, L_4}
    {1, SW10_CS5, SW11_CS5, SW12_CS5}, // {1, J_5, K_5, L_5}
    {1, SW10_CS8, SW11_CS8, SW12_CS8}, // {1, J_8, K_8, L_8}

    // Row 6
    {0, SW10_CS9, SW11_CS9, SW12_CS9},    // {0, J_9, K_9, L_9}
    {0, SW10_CS10, SW11_CS10, SW12_CS10}, // {0, J_10, K_10, L_10}
    {0, SW10_CS11, SW11_CS11, SW12_CS11}, // {0, J_11, K_11, L_11}
    {0, SW10_CS12, SW11_CS12, SW12_CS12}, // {0, J_12, K_12, L_12}
    {1, SW10_CS6, SW11_CS6, SW12_CS6},    // {1, J_6, K_6, L_6}
    {1, SW10_CS7, SW11_CS7, SW12_CS7},    // {1, J_7, K_7, L_7}
    {1, SW10_CS9, SW11_CS9, SW12_CS9},    // {1, J_9, K_9, L_9}
    {1, SW10_CS10, SW11_CS10, SW12_CS10}, // {1, J_10, K_10, L_10}
    {1, SW10_CS11, SW11_CS11, SW12_CS11}, // {1, J_11, K_11, L_11}

    // Row 7 (Looks like a partial row or common LEDs?)
    {0, SW1_CS1, SW2_CS1, SW3_CS1}, // {0, A_1, B_1, C_1}
    {0, SW1_CS2, SW2_CS2, SW3_CS2}, // {0, A_2, B_2, C_2}
    {0, SW1_CS3, SW2_CS3, SW3_CS3}, // {0, A_3, B_3, C_3}
    {0, SW1_CS4, SW2_CS4, SW3_CS4}, // {0, A_4, B_4, C_4}
    {0, SW1_CS5, SW2_CS5, SW3_CS5}, // {0, A_5, B_5, C_5}

    // Row 8
    {1, SW10_CS12, SW11_CS12, SW12_CS12}, // {1, J_12, K_12, L_12}
    {1, SW10_CS13, SW11_CS13, SW12_CS13}, // {1, J_13, K_13, L_13}
    {1, SW10_CS14, SW11_CS14, SW12_CS14}, // {1, J_14, K_14, L_14}
    {1, SW10_CS15, SW11_CS15, SW12_CS15}, // {1, J_15, K_15, L_15}
    {1, SW10_CS16, SW11_CS16, SW12_CS16}, // {1, J_16, K_16, L_16}

    // Row 9 (Longest row)
    {0, SW4_CS12, SW5_CS12, SW6_CS12},    // {0, D_12, E_12, F_12}
    {0, SW4_CS13, SW5_CS13, SW6_CS13},    // {0, D_13, E_13, F_13}
    {0, SW4_CS14, SW5_CS14, SW6_CS14},    // {0, D_14, E_14, F_14}
    {0, SW4_CS15, SW5_CS15, SW6_CS15},    // {0, D_15, E_15, F_15}
    {0, SW4_CS16, SW5_CS16, SW6_CS16},    // {0, D_16, E_16, F_16}
    {0, SW7_CS16, SW8_CS16, SW9_CS16},    // {0, G_16, H_16, I_16}
    {0, SW7_CS15, SW8_CS15, SW9_CS15},    // {0, G_15, H_15, I_15}
    {0, SW7_CS14, SW8_CS14, SW9_CS14},    // {0, G_14, H_14, I_14}
    {0, SW7_CS13, SW8_CS13, SW9_CS13},    // {0, G_13, H_13, I_13}
    {0, SW7_CS12, SW8_CS12, SW9_CS12},    // {0, G_12, H_12, I_12}
    {0, SW7_CS11, SW8_CS11, SW9_CS11},    // {0, G_11, H_11, I_11}
    {0, SW7_CS10, SW8_CS10, SW9_CS10},    // {0, G_10, H_10, I_10}
    {0, SW7_CS9, SW8_CS9, SW9_CS9},       // {0, G_9, H_9, I_9}
    {0, SW10_CS16, SW11_CS16, SW12_CS16}, // {0, J_16, K_16, L_16}
    {0, SW10_CS15, SW11_CS15, SW12_CS15}, // {0, J_15, K_15, L_15}
    {0, SW10_CS14, SW11_CS14, SW12_CS14}, // {0, J_14, K_14, L_14}
    {0, SW10_CS13, SW11_CS13, SW12_CS13}, // {0, J_13, K_13, L_13}
    {1, SW4_CS16, SW5_CS16, SW6_CS16},    // {1, D_16, E_16, F_16}
    {1, SW4_CS15, SW5_CS15, SW6_CS15},    // {1, D_15, E_15, F_15}
    {1, SW4_CS14, SW5_CS14, SW6_CS14},    // {1, D_14, E_14, F_14}
    {1, SW4_CS13, SW5_CS13, SW6_CS13},    // {1, D_13, E_13, F_13}
    {1, SW4_CS12, SW5_CS12, SW6_CS12},    // {1, D_12, E_12, F_12}
    {1, SW4_CS11, SW5_CS11, SW6_CS11},    // {1, D_11, E_11, F_11}
    {1, SW4_CS10, SW5_CS10, SW6_CS10},    // {1, D_10, E_10, F_10}
    {1, SW4_CS9, SW5_CS9, SW6_CS9},       // {1, D_9, E_9, F_9}
    {1, SW7_CS7, SW8_CS7, SW9_CS7},       // {1, G_7, H_7, I_7}
    {1, SW7_CS8, SW8_CS8, SW9_CS8},       // {1, G_8, H_8, I_8}
    {1, SW7_CS9, SW8_CS9, SW9_CS9},       // {1, G_9, H_9, I_9}
    {1, SW7_CS10, SW8_CS10, SW9_CS10},    // {1, G_10, H_10, I_10}
    {1, SW7_CS11, SW8_CS11, SW9_CS11},    // {1, G_11, H_11, I_11}
    {1, SW7_CS12, SW8_CS12, SW9_CS12},    // {1, G_12, H_12, I_12}
    {1, SW7_CS13, SW8_CS13, SW9_CS13},    // {1, G_13, H_13, I_13}
    {1, SW7_CS14, SW8_CS14, SW9_CS14},    // {1, G_14, H_14, I_14}
    {1, SW7_CS15, SW8_CS15, SW9_CS15},    // {1, G_15, H_15, I_15}
    {1, SW7_CS16, SW8_CS16, SW9_CS16}     // {1, G_16, H_16, I_16}
};


/**
 * @brief  gpio initial.
 */
void m_gpio_init(void)
{
    setPinOutput(DC_BOOST_PIN); writePinHigh(DC_BOOST_PIN);

    // Initializes the RGB Driver SDB pin
    setPinOutput(RGB_DRIVER_SDB1); writePinHigh(RGB_DRIVER_SDB1);
    setPinOutput(RGB_DRIVER_SDB2); writePinHigh(RGB_DRIVER_SDB2);

    // RF wake up pin configuration
    setPinOutput(NRF_WAKEUP_PIN);
    writePinHigh(NRF_WAKEUP_PIN);

    // RFboot Control pin
    setPinInputHigh(NRF_BOOT_PIN);

    // RF reset pin configuration
    setPinOutput(NRF_RESET_PIN); writePinLow(NRF_RESET_PIN);
    wait_ms(50);
    writePinHigh(NRF_RESET_PIN);

    // Switch detection pin
    setPinInputHigh(DEV_MODE_PIN);
    setPinInputHigh(SYS_MODE_PIN);
}


/**
 * @brief  long press key process.
 */
void long_press_key(void)
{
    static uint32_t long_press_timer = 0;

    if (timer_elapsed32(long_press_timer) < 100) return;
    long_press_timer = timer_read32();

    if (f_rf_sw_press) {
        rf_sw_press_delay++;
        if (rf_sw_press_delay >= RF_LONG_PRESS_DELAY) {
            f_rf_sw_press        = 0;
            dev_info.link_mode   = rf_sw_temp;
            dev_info.rf_channel  = rf_sw_temp;
            dev_info.ble_channel = rf_sw_temp;

            uint8_t timeout = 5;
            while (timeout--) {
                uart_send_cmd(CMD_NEW_ADV, 0, 1); 
                wait_ms(20);                     
                uart_receive_pro();
                if (f_rf_new_adv_ok) break;
            }
        }
    } else {
        rf_sw_press_delay = 0;
    }

    if (f_dev_reset_press) {
        dev_reset_press_delay++;
        if (dev_reset_press_delay >= DEV_RESET_PRESS_DELAY)  {
            f_dev_reset_press = 0;

            if (dev_info.link_mode != LINK_USB) {
                if (dev_info.link_mode != LINK_RF_24) {
                    dev_info.link_mode      = LINK_BT_1;
                    dev_info.ble_channel    = LINK_BT_1;
                    dev_info.rf_channel     = LINK_BT_1;
                }
            } else {
                dev_info.ble_channel = LINK_BT_1;
            }

            uart_send_cmd(CMD_SET_LINK, 10, 10);   
            wait_ms(500);                           
            uart_send_cmd(CMD_CLR_DEVICE, 10, 10); 

            eeconfig_init();     
            device_reset_show(); 
            device_reset_init(); 

            keymap_config.no_gui = 0;
            f_win_lock = 0;

            if (dev_info.sys_sw_state == SYS_SW_MAC) {
                default_layer_set(1 << 0);  // MAC
                keymap_config.nkro = 0;     
            } else {
                default_layer_set(1 << 2);  // WIN
                keymap_config.nkro = 1;   
            }
        }
    } else {
        dev_reset_press_delay = 0;
    }

    if (f_rgb_test_press) {
        rgb_test_press_delay++;
        if (rgb_test_press_delay >= RGB_TEST_PRESS_DELAY) {
            f_rgb_test_press = 0;
            rgb_test_show();  
        }
    } else {
        rgb_test_press_delay = 0;
    }
}

/**
 * @brief  Release all keys, clear keyboard report.
 */
void m_break_all_key(void)
{
    
    uint8_t report_buf[16];
    bool nkro_temp = keymap_config.nkro; 

    clear_weak_mods();
    clear_mods();
    clear_keyboard();

    keymap_config.nkro = 1;
    memset(nkro_report, 0, sizeof(report_nkro_t));
    host_nkro_send(nkro_report);
    wait_ms(10);

    keymap_config.nkro = 0;
    memset(keyboard_report, 0, sizeof(report_keyboard_t));
    host_keyboard_send(keyboard_report);
    wait_ms(10);

    keymap_config.nkro = nkro_temp;

    if (dev_info.link_mode != LINK_USB) {
        memset(report_buf, 0, 16);
        uart_send_report(CMD_RPT_BIT_KB, report_buf, 16);
        wait_ms(10);
        uart_send_report(CMD_RPT_BYTE_KB, report_buf, 8);
        wait_ms(10);
    }

    memset(uart_bit_report_buf, 0, sizeof(uart_bit_report_buf));
    memset(bitkb_report_buf, 0, sizeof(bitkb_report_buf));
    memset(bytekb_report_buf, 0, sizeof(bytekb_report_buf));
}

/**
 * @brief  switch device link mode.
 * @param mode : link mode
 */
static void switch_dev_link(uint8_t mode)
{
    if (mode > LINK_USB) return;
    m_break_all_key();

    dev_info.link_mode = mode;
    dev_info.rf_state = RF_IDLE;
    f_send_channel    = 1;

    if (mode == LINK_USB) {
        host_mode = HOST_USB_TYPE;
        host_set_driver(m_host_driver);
        rf_link_show_time = 0;
    }
    else {
        host_mode = HOST_RF_TYPE;
        host_set_driver(&rf_host_driver);
    }
}


/**
 * @brief  scan dial switch.
 */
void dial_sw_scan(void)
{
    uint8_t dial_scan               = 0;
    static uint8_t dial_save        = 0xf0;
    static uint8_t debounce         = 0;
    static uint32_t dial_scan_timer = 0;
    static bool flag_power_on       = 1;

    if (!flag_power_on) {
        if (timer_elapsed32(dial_scan_timer) < 20) return;
    }
    dial_scan_timer = timer_read32();

    setPinInputHigh(DEV_MODE_PIN);
    setPinInputHigh(SYS_MODE_PIN);

    if (readPin(DEV_MODE_PIN)) dial_scan |= 0X01;
    if (readPin(SYS_MODE_PIN)) dial_scan |= 0X02;

    if (dial_save != dial_scan) {
        m_break_all_key();          
        dial_save         = dial_scan;
        no_act_time         = 0;   
        rf_linking_time     = 0;  
        debounce            = 25;  
        f_dial_sw_init_ok   = 0;  
        return;
    } else if (debounce) {
        debounce--;
        return;
    }

    if (dial_scan & 0x01) {
        if (dev_info.link_mode != LINK_USB) {
            switch_dev_link(LINK_USB);
        }
    } else {
        if (dev_info.link_mode != dev_info.rf_channel) {
            switch_dev_link(dev_info.rf_channel);
        }
    }

    if (dial_scan & 0x02) {
        if (dev_info.sys_sw_state != SYS_SW_WIN) {
            f_sys_show = 1;
            default_layer_set(1 << 2);  
            dev_info.sys_sw_state = SYS_SW_WIN;
            keymap_config.no_gui  = f_win_lock;
            m_break_all_key();  
        }
        keymap_config.nkro = 1; 
    } else {
        if (dev_info.sys_sw_state != SYS_SW_MAC) {
            f_sys_show = 1;
            default_layer_set(1 << 0);
            dev_info.sys_sw_state = SYS_SW_MAC;
            f_win_lock            = keymap_config.no_gui;
            m_break_all_key();  
        }
        keymap_config.nkro   = 0;
        keymap_config.no_gui = 0;
    }

    if (f_dial_sw_init_ok == 0) {
        f_dial_sw_init_ok = 1;  
        flag_power_on     = 0;

        if (dev_info.link_mode != LINK_USB) {
            host_set_driver(&rf_host_driver);
        }
    }
}

/**
 * @brief  power on scan dial switch.
 */
void m_power_on_dial_sw_scan(void)
{
    uint8_t dial_scan_dev = 0;
    uint8_t dial_scan_sys = 0;
    uint8_t dial_check_dev = 0;
    uint8_t dial_check_sys = 0;
    uint8_t debounce = 0;

    f_win_lock = 0;

    setPinInputHigh(DEV_MODE_PIN);   
    setPinInputHigh(SYS_MODE_PIN);  

    for(debounce=0; debounce<10; debounce++) {
        dial_scan_dev = 0;
        dial_scan_sys = 0;
        if (readPin(DEV_MODE_PIN))  dial_scan_dev = 0x01;
        else                        dial_scan_dev = 0;
        if (readPin(SYS_MODE_PIN))  dial_scan_sys = 0x01;
        else                        dial_scan_sys = 0;
        if((dial_scan_dev != dial_check_dev)||(dial_scan_sys != dial_check_sys))
        {
            dial_check_dev = dial_scan_dev;
            dial_check_sys = dial_scan_sys;
            debounce = 0;
        }
        wait_ms(1);
    }
    if (dial_scan_dev) {
        if (dev_info.link_mode != LINK_USB) {
            switch_dev_link(LINK_USB);
        }
    } else {
        if (dev_info.link_mode != dev_info.rf_channel) {
            switch_dev_link(dev_info.rf_channel);
        }
    }
    // WIN/MAC 
    if (dial_scan_sys) {
        if (dev_info.sys_sw_state != SYS_SW_WIN) {
            default_layer_set(1 << 2);  // WIN
            dev_info.sys_sw_state = SYS_SW_WIN;
            keymap_config.nkro    = 1;  
            m_break_all_key();  
        }
    } else {
        if (dev_info.sys_sw_state != SYS_SW_MAC) {
            default_layer_set(1 << 0);  // MAC
            dev_info.sys_sw_state = SYS_SW_MAC;
            keymap_config.nkro    = 0; 
            f_win_lock            = keymap_config.no_gui;
            keymap_config.no_gui  = 0;
            m_break_all_key();  
        }
    }
}


/**
 * @brief  qmk process record
 */
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if(!process_record_user(keycode, record)){
        return false;
    }
    no_act_time = 0;
    switch (keycode) {
        case RF_DFU:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) return false;
                uart_send_cmd(CMD_RF_DFU, 10, 20); 
            }
            return false;

        case LNK_USB:
            if (record->event.pressed) {
                m_break_all_key();
            } else {
                dev_info.link_mode = LINK_USB;
                uart_send_cmd(CMD_SET_LINK, 10, 10);
            }
            return false;

        case LNK_RF:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) {
                    rf_sw_temp    = LINK_RF_24;
                    f_rf_sw_press = 1;
                    m_break_all_key();
                }
            } else if (f_rf_sw_press) {
                f_rf_sw_press = 0;
                if (rf_sw_press_delay < RF_LONG_PRESS_DELAY) {
                    dev_info.link_mode   = rf_sw_temp;
                    dev_info.rf_channel  = rf_sw_temp;
                    dev_info.ble_channel = rf_sw_temp;
                    uart_send_cmd(CMD_SET_LINK, 10, 20);  
                }
            }
            return false;

        case LNK_BLE1:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) {
                    rf_sw_temp    = LINK_BT_1;
                    f_rf_sw_press = 1;
                    m_break_all_key();
                }
            } else if (f_rf_sw_press) {
                f_rf_sw_press = 0;
                if (rf_sw_press_delay < RF_LONG_PRESS_DELAY) {
                    dev_info.link_mode   = rf_sw_temp;
                    dev_info.rf_channel  = rf_sw_temp;
                    dev_info.ble_channel = rf_sw_temp;
                    uart_send_cmd(CMD_SET_LINK, 10, 20); 
                }
            }
            return false;

        case LNK_BLE2:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) {
                    rf_sw_temp    = LINK_BT_2;
                    f_rf_sw_press = 1;
                    m_break_all_key();
                }
            } else if (f_rf_sw_press) {
                f_rf_sw_press = 0;
                if (rf_sw_press_delay < RF_LONG_PRESS_DELAY) {
                    dev_info.link_mode   = rf_sw_temp;
                    dev_info.rf_channel  = rf_sw_temp;
                    dev_info.ble_channel = rf_sw_temp;
                    uart_send_cmd(CMD_SET_LINK, 10, 20);  
                }
            }
            return false;

        case LNK_BLE3:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) {
                    rf_sw_temp    = LINK_BT_3;
                    f_rf_sw_press = 1;
                    m_break_all_key();
                }
            } else if (f_rf_sw_press) {
                f_rf_sw_press = 0;
                if (rf_sw_press_delay < RF_LONG_PRESS_DELAY) {
                    dev_info.link_mode   = rf_sw_temp;
                    dev_info.rf_channel  = rf_sw_temp;
                    dev_info.ble_channel = rf_sw_temp;
                    uart_send_cmd(CMD_SET_LINK, 10, 20); 
                }
            }
            return false;

        case MAC_TASK:
            if (record->event.pressed) {
                host_consumer_send(0x029F);
            } else {
                host_consumer_send(0);
            }
            return false;

        case MAC_SEARCH:
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_SPACE);
                wait_ms(20);
                unregister_code(KC_LGUI);
                unregister_code(KC_SPACE);
            }
            return false;

        case MAC_VOICE:
            if (record->event.pressed) {
                host_consumer_send(0xcf);
            } else {
                host_consumer_send(0);
            }
            return false;

        case MAC_CONSOLE:
            if (record->event.pressed) {
                host_consumer_send(0x02A0);
            } else {
                host_consumer_send(0);
            }
            return false;

        case MAC_DND:
            if (record->event.pressed) {
                host_system_send(0x9b);
            } else {
                host_system_send(0);
            }
            return false;

        case MAC_PRT:
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_LSFT);
                register_code(KC_3);
                wait_ms(20);
                unregister_code(KC_3);
                unregister_code(KC_LSFT);
                unregister_code(KC_LGUI);
            }
            return false;

        case MAC_PRTA:
            if (record->event.pressed) {
                // win
                if (keymap_config.nkro) {
                    register_code(KC_LGUI);
                    register_code(KC_LSFT);
                    register_code(KC_S);
                    wait_ms(20);
                    unregister_code(KC_S);
                    unregister_code(KC_LSFT);
                    unregister_code(KC_LGUI);
                }
                // mac
                else {
                    register_code(KC_LGUI);
                    register_code(KC_LSFT);
                    register_code(KC_4);
                    wait_ms(20);
                    unregister_code(KC_4);
                    unregister_code(KC_LSFT);
                    unregister_code(KC_LGUI);
                }
            }
            return false;

        case SIDE_VAI:
            if (record->event.pressed) {
                if(low_bat_flag && (side_light == 1)) return false;
                light_level_control(1);
            }
            return false;

        case SIDE_VAD:
            if (record->event.pressed) {
                light_level_control(0);
            }
            return false;

        case SIDE_MOD_A:
            if (record->event.pressed) {
                side_mode_a_control(1);
            }
            return false;

        case SIDE_MOD_B:
            if (record->event.pressed) {
                side_mode_b_control(1);
            }
            return false;

        case SIDE_HUI:
            if (record->event.pressed) {
                side_colour_control(1);
            }
            return false;

        case SIDE_SPI:
            if (record->event.pressed) {
                light_speed_control(1);
            }
            return false;

        case SIDE_SPD:
            if (record->event.pressed) {
                light_speed_control(0);
            }
            return false;

        case DEV_RESET:
            if (record->event.pressed) {
                f_dev_reset_press = 1;
                m_break_all_key(); 
            } else {
                f_dev_reset_press = 0;
            }
            return false;

        case SLEEP_MODE:
            if (record->event.pressed) {
                if(f_dev_sleep_enable) f_dev_sleep_enable = false;
                else f_dev_sleep_enable = true;
                f_sleep_show       = 1;
                eeconfig_update_user_datablock(&user_config, 0, sizeof(user_config));  
            }
            return false;

        case BAT_SHOW:
            if (record->event.pressed) {
                f_bat_hold = !f_bat_hold;
            }
            return false;

        case RGB_VAI:
            if(low_bat_flag && (rgb_matrix_config.hsv.v == RGB_MATRIX_VAL_STEP)) return false;
            return true;

        case RGB_TOG:
            if (record->event.pressed) {
                rgb_matrix_enable();  
                if(rgb_matrix_config.hsv.v)
                {
                    rgb_light_old = rgb_matrix_config.hsv.v;
                    rgb_matrix_config.hsv.v = 0;
                }
                else
                {
                    if(rgb_light_old) rgb_matrix_config.hsv.v = rgb_light_old;
                    else rgb_matrix_config.hsv.v = (255 - RGB_MATRIX_SPD_STEP * 2);
                }
                 
            }
            return false;
            
        default:
            return true;
    }
    return true;
}


/**
    @brief  timer process.
 */
void timer_pro(void)
{
    static uint32_t interval_timer = 0;
    static bool f_first            = true;

    if (f_first) {
        f_first        = false;
        interval_timer = timer_read32();
        m_host_driver  = host_get_driver();
    }

    if (timer_elapsed32(interval_timer) < 10) {
        return;
    } else if (timer_elapsed32(interval_timer) > 20) {
        interval_timer = timer_read32();
    } else {
        interval_timer += 10;  
    }

    if (rf_link_show_time < RF_LINK_SHOW_TIME)
        rf_link_show_time++;

    if (no_act_time < 0xffff)
        no_act_time++;

    if (rf_linking_time < 0xffff)
        rf_linking_time++;

}


/**
 * @brief  londing eeprom data.
 */
void m_londing_eeprom_data(void)
{
    eeconfig_read_user_datablock(&user_config, 0, sizeof(user_config));
    if (user_config.default_brightness_flag != 0xA5) {
        rgb_matrix_sethsv(RGB_DEFAULT_COLOUR, 255, RGB_MATRIX_MAXIMUM_BRIGHTNESS - RGB_MATRIX_VAL_STEP * 2);  
        user_config.default_brightness_flag = 0xA5;
        user_config.ee_side_mode_a           = side_mode_a;
        user_config.ee_side_mode_b           = side_mode_b;
        user_config.ee_side_light           = side_light;
        user_config.ee_side_speed           = side_speed;
        user_config.ee_side_rgb             = side_rgb;
        user_config.ee_side_colour          = side_colour;
        f_dev_sleep_enable                  = true;
        eeconfig_update_user_datablock(&user_config, 0, sizeof(user_config));  
    } else {
        side_mode_a   = user_config.ee_side_mode_a;
        side_mode_b   = user_config.ee_side_mode_b;
        side_light  = user_config.ee_side_light;
        side_speed  = user_config.ee_side_speed;
        side_rgb    = user_config.ee_side_rgb;
        side_colour = user_config.ee_side_colour;
    }
}


/**
   qmk keyboard post init
 */
void keyboard_post_init_kb(void)
{
    m_gpio_init();      
    rf_uart_init();
    wait_ms(500);
    rf_device_init();

    m_break_all_key();
    m_londing_eeprom_data();
    m_power_on_dial_sw_scan();
    keyboard_post_init_user();

    rf_link_show_time = 0;
}

/**
   rgb_matrix_indicators_user
 */
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max)
{
    if (keymap_config.no_gui) {
        rgb_matrix_set_color(72, 0x00, 0x80, 0x00);
    }
    return true;
}

/**
   housekeeping_task_kb
 */
void housekeeping_task_kb(void)
{

    timer_pro();

    uart_receive_pro();

    uart_send_report_func();

    dev_sts_sync();

    long_press_key();

    dial_sw_scan();

    m_side_led_show();

    Sleep_Handle();

}
