/*
Copyright 2023 NuPhy & jincao1

Adapted from ryodeushii/qmk-firmware
(keyboards/nuphy/common/power/mcu_pwr.h) for the upstream QMK port.

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

#pragma once

#include <stdbool.h>

void enter_light_sleep(void);
void exit_light_sleep(void);
void enter_deep_sleep(void);
void exit_deep_sleep(void);

bool pwr_is_sleeping(void);

void led_pwr_sleep_handle(void);
void led_pwr_wake_handle(void);

void pwr_rgb_led_off(void);
void pwr_rgb_led_on(void);

bool is_rgb_led_on(void);
