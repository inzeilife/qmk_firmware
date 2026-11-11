/*
Copyright 2024 @ jincao1, inspired by Keychron

Adapted from ryodeushii/qmk-firmware
(keyboards/nuphy/common/wireless/rf_queue.h) for the upstream QMK port.

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
#include <stdint.h>

#define RF_REPORT_BUFFER_SIZE 16

typedef struct {
    uint8_t cmd;
    uint8_t length;
    uint8_t buffer[RF_REPORT_BUFFER_SIZE];
} rf_report_t;

bool rf_queue_enqueue(const rf_report_t *report);
bool rf_queue_dequeue(rf_report_t *report);
bool rf_queue_is_empty(void);
void rf_queue_clear(void);
