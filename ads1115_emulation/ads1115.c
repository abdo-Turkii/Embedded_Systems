#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/critical_section.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"

#define PIN_BUTTON 24
#define ADC_26 26
#define ADC_27 27
#define ADC_28 28
#define ADC_29 29

static const uint I2C_SLAVE_ADDRESS = 0x48;
static const uint I2C_BAUDRATE = 100*1000; // 100 kHz 
static uint active_adc = 0;
static uint fsr = 2;
static double gain ,result ;
critical_section_t myLock;

#ifdef i2c_default
// For this program, we emulat ADS1115 by using RP2040.
// You'll need to wire pin GPIO 18 (SDA), and pin GPIO 19 (SCL).
static const uint I2C_SLAVE_SDA_PIN = 18; 
static const uint I2C_SLAVE_SCL_PIN = 19; 

// The slave implements a 256 byte memory. To write a series of bytes, the master first
// writes the memory address, followed by the data.
static uint16_t buffer[4]={56,24,54,44};
static uint8_t buffer_index = 0  ;   //refar to the number of register
static bool buffer_index_written ;   // acknowledge bit

double get_adc_read()
{
    buffer[0] = adc_read();
    result = (buffer[0]) / 4095 ;
    result = result * gain ;
}

void adc_config()
{
    // Initialise the ADC. reset clock for ABC 
    adc_init();                                 
    adc_gpio_init(ADC_26);                   // GPIO 26 use as ADC pin
    adc_gpio_init(ADC_27);                  // GPIO 27 use as ADC pin
    adc_gpio_init(ADC_28);                 // GPIO 28 use as ADC pin
    adc_gpio_init(ADC_29);                // GPIO 29 use as ADC pin  

}
void adc_settings()
{
    //Select adc pin input from  pin 0 to 3
    active_adc = (buffer[0]>> 12) & 3;
    adc_select_input(active_adc); 

    //Seleect PAG grom FSR = 0.256 to 6.144 V
    fsr = (buffer[0]>> 8) & 7;
    if((fsr == 7) ||(fsr == 6) || (fsr == 5) ){gain = 0.256 ;}
    else if(fsr == 4){gain = 0.512 ;}
    else if(fsr == 3){gain = 1.024 ;}
    else if(fsr == 2){gain = 2.048 ;}
    else if(fsr == 1){gain = 4.055 ;}
    else if(fsr == 0){gain = 6.144 ;}
}

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    //critical zone start
    critical_section_enter_blocking(&myLock);
    
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!buffer_index_written) {
            // writes always start with the memory address
            buffer_index = i2c_read_byte_raw(i2c); 
            buffer_index_written = true;
        } else {
            // save into memory
            buffer[buffer_index] = i2c_read_byte_raw(i2c);
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data load from ADC
        get_adc_read();
        i2c_write_byte_raw(i2c, result);
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        buffer_index_written = false;
        adc_settings();
        break;
    default:
        break;
    }
    //critical zone end
    critical_section_exit(&myLock);
}

static void setup_slave() {
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
}
#endif

int main() {
    stdio_init_all();
    
    //initilisation ADC chanels 
    adc_config();
    //initilaization of critical section
    critical_section_init(&myLock);

    //set interrput priority
    irq_set_priority(I2C0_IRQ,0x00);   //Highest priority of irq

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c / slave_mem_i2c example requires a board with I2C pins
    puts("Default I2C pins were not defined");
    return 0;
#else

    //operating function 
    setup_slave();
    while (true);
   
#endif
}