SRC += side.c rf.c sleep.c rf_driver.c

OPT ?= 2
CUSTOM_MATRIX = lite
SRC += matrix.c debounce.c

UART_DRIVER_REQUIRED   = yes
RGB_MATRIX_CUSTOM_USER = yes
