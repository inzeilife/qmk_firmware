/*
Copyright 2024 @ jincao1, inspired by Keychron

Adapted from ryodeushii/qmk-firmware
(keyboards/nuphy/common/wireless/rf_queue.c) for the upstream QMK port.

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

#include "rf_queue.h"

#define RF_QUEUE_SIZE 32

static rf_report_t queue[RF_QUEUE_SIZE];
static uint8_t     queue_head = 0;
static uint8_t     queue_tail = 0;

bool rf_queue_is_empty(void) {
    return queue_head == queue_tail;
}

void rf_queue_clear(void) {
    queue_head = 0;
    queue_tail = 0;
}

bool rf_queue_enqueue(const rf_report_t *report) {
    uint8_t next = (queue_head + 1) % RF_QUEUE_SIZE;

    if (next == queue_tail) {
        return false;
    }

    queue[queue_head] = *report;
    queue_head        = next;
    return true;
}

bool rf_queue_dequeue(rf_report_t *report) {
    if (rf_queue_is_empty()) {
        return false;
    }

    *report    = queue[queue_tail];
    queue_tail = (queue_tail + 1) % RF_QUEUE_SIZE;
    return true;
}
