#ifndef KEYPAD_H
#define KEYPAD_H

#include "pico/stdlib.h"
#include <stdio.h>
#include <stdint.h>
#include "hardware/i2c.h"
#include <hardware/gpio.h>
#include "lcd.h"

// Define the rows and columns of the keypad
#define ROWS 4
#define COLS 4
#define LOW 0
#define HIGH 1

void init_gpio();
char scan_keypad();

#endif