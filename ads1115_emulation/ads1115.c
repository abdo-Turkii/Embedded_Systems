#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/critical_section.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"

// ADC channels
#define ADC_26 26
#define ADC_27 27
#define ADC_28 28
#define ADC_29 29
static uint active_adc = 0;
static double gain = 0.00 , result = 0.00;
static uint fsr = 2;
static uint16_t config ;
// I2C address and pins
static const uint I2C_SLAVE_ADDRESS = 0x055;
static const uint I2C_BAUDRATE = 100*1000; // 100 kHz 

//define critical section lock
critical_section_t myLock;

#ifdef i2c1
// For this program, we emulat ADS1115 by using RP2040.
// You'll need to wire pin GPIO 18 (SDA), and pin GPIO 19 (SCL).
static const uint I2C_SLAVE_SDA_PIN = 18; 
static const uint I2C_SLAVE_SCL_PIN = 19; 

// The slave implements a 256 byte memory. To write a series of bytes, the master first
// writes the memory address, followed by the data.
static uint8_t buffer[4]={0,0,0,0};
static uint8_t buffer_index = 4  ;   //refar to the number of register
static bool buffer_index_written ;   // acknowledge bit

void get_adc_read()
{
    buffer[0] = adc_read();
    buffer[0] = (buffer[0] << 3 ) ;  
    //buffer[0] = (buffer[0] * 32767) / 4095 ;         // store 12 bit in 16 bit
    //result = ((buffer[0] * 3.3) / 65535) *(6.144/3.3);    // convert result into volt
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
    active_adc = (config >> 12) & 3;
    adc_select_input(active_adc); 

    //Seleect PAG from FSR = 0.256 to 6.144 V
    fsr = (config >> 8) & 7;
    if(fsr >= 5)     {gain = 0.256 / 3.3 ;}
    else if(fsr == 4){gain = 0.512 / 3.3 ;}
    else if(fsr == 3){gain = 1.024 / 3.3 ;}
    else if(fsr == 2){gain = 2.048 / 3.3;}
    else if(fsr == 1){gain = 4.055 / 3.3;}
    else if(fsr == 0){gain = 6.144 / 3.3;}
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
            printf("0: %u\n",buffer_index);

        } else {
            // save into configration register 
            buffer[buffer_index]  = i2c_read_byte_raw(i2c);
            printf("00: %u\n",buffer_index);
            printf("1: %u\n",buffer[buffer_index]);
            config = (buffer_index << 8) | buffer[buffer_index];
            printf("Settings: %u\n",config );

      
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data load from ADC
        get_adc_read();
        printf("THE value is: %u\n",buffer[0]);
        i2c_write_byte_raw(i2c, (buffer[buffer_index] >> 8) );
        i2c_write_byte_raw(i2c, buffer[buffer_index] );
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        buffer_index_written = false;
        //adc_settings();
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

    i2c_init(i2c1, I2C_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c1, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
}
#endif

int main() {
    stdio_init_all();
    
    //initilisation ADC channels 
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