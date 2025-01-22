#ifndef DEFINITION_H
#define DEFINITION_H

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/critical_section.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "pca.h"
#include "ads.h"

// I2C address and baudrate
// Address of i2c slave in RP2040
#define I2C_SLAVE_ADDRESS  0x055    
// 400 kHz for i2c bus data rate transfer    
#define I2C_BAUDRATE    400*1000   

// Initialize I2C0 bus as a slave
void setup_i2c0_slave();

// Initialize I2C1 bus as a slave
void setup_i2c1_slave();

// Function to initialize ADC GPIO
void adc_config();

//Function to get signal and convert it to 15 bits.
void get_adc_read();

//Function to apply configuration 
void adc_settings();

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c0_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);

//Set default value of registers        
void init_pca9685();

// Function to configuration GPIO as PWM 
void pwm_configuration(uint8_t channel_number , uint16_t led_on, uint16_t led_off);

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c1_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);

#endif