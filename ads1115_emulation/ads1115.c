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
// For this program, i try to  emulate ADS1115 by using RP2040.
// from RP2040 i using GPIO(26,27,28,29) as a ADC converter with 12 bits resolution. 
// ADS1115 have a 16 bits resolution so we convert 12 bits to 16 bits to send data in 16 bits 
// Datasheet of ADS1115 [https://www.ti.com/lit/ds/symlink/ads1115.pdf].
// Git hub link [].
static bool config_flag;
// From data sheet of ADS115 section [9.6.3] Config Register Field Descriptions 
// Table(8) Config Register Field Descriptions.
struct config_register {
    unsigned int comp_que : 2;      //Comparator queue and disable      
    unsigned int comp_lat : 1;     //This bit controls whether the ALERT/RDY pin latches
    unsigned int comp_pol : 1;    //This bit controls the polarity of the ALERT/RDY pin
    unsigned int comp_mode: 1;   //This bit configures the comparator operating mode.
    unsigned int data_rate: 3;  //These bits control the data rate setting.
    unsigned int mode: 1;      //This bit controls the operating mode.
    unsigned int pga:  3;     //These bits set the FSR of the programmable gain amplifier
    unsigned int mux: 3;     //These bits configure the input multiplexer.
    unsigned int os: 1;     //Operational status or single-shot conversion start
} config;

// GPIO on rp2040 for ADC channels 
#define ADC_26 26
#define ADC_27 27
#define ADC_28 28
#define ADC_29 29

// I2C address and pins
#define I2C_SLAVE_ADDRESS  0x055   //address of i2c slave in RP2040
#define I2C_BAUDRATE  400*1000    // 400 kHz for i2c bus datarate transfer

// You'll need to wire pin GPIO 18 (SDA), and pin GPIO 19 (SCL).
#define I2C_SLAVE_SDA_PIN 18
#define I2C_SLAVE_SCL_PIN 19 

// From data sheet of ADS115 section [9.6] Register Map for ADS1115  
// Table (6). Address Pointer Register Field Descriptions
static uint16_t set_configration =0x8483  ,lo_thresh_reg =0x8000,ho_thresh_reg=0x7FFF  ;
static uint8_t add_ptr_reg = 0;             // add_ptr_reg refar to the number of register
static uint8_t msb = 0x84 , lsb = 0x83 ;   // 2 bytes of data come from master i2c bus
uint16_t max_range ;                      // max range for input signal
uint16_t input_signal ,response ;                 // variable for signal readed by RP2040 
//define critical section lock
critical_section_t myLock;

#ifdef i2c1
// Function to initialize ADC GPIO
void adc_config()
{
    // Initialise the ADC. reset clock for ABC 
    adc_init();                                 
    adc_gpio_init(ADC_26);                   // GPIO 26 use as ADC pin
    adc_gpio_init(ADC_27);                  // GPIO 27 use as ADC pin
    adc_gpio_init(ADC_28);                 // GPIO 28 use as ADC pin
    adc_gpio_init(ADC_29);                // GPIO 29 use as ADC pin  
}
//Function to get signal and convert it to 15 bits.
void get_adc_read()
{
    input_signal = adc_read();               //get signal from ADC after convertion it to digital
    if(input_signal >= max_range)           //saturate signal 
        response = 32767;                //MAX in range . 
    else
        response = (input_signal * 32767 ) / max_range ;   //Store signal in 15 bits 
}

//Function to apply configration 
void adc_settings()
{
    //Select adc pin input from  pin 0 to 3 (Gpio from 26 to 29)
    adc_select_input(config.mux % 4); 

    //From datas sheet of ADS1115 insection [9.3.3] Full-Scale Range (FSR) and LSB Size
    //Table (3). Full-Scale Range and Corresponding LSB Size
    //Seleect PGA from [FSR = 0.256 to 6.144]V
    if(config.pga >= 5 )    {max_range   = 328     ;}  // get read in range from 0 to 256  mV
    else if(config.pga == 4){max_range   = 635     ;}  // get read in range from 0 to 512  mV
    else if(config.pga == 3){max_range   = 1271    ;}  // get read in range from 0 to 1024 mV
    else if(config.pga == 2){max_range   = 2542    ;}  // get read in range from 0 to 2048 mV
    else if(config.pga == 1){max_range   = 5084    ;}  // get read in range from 0 to 4096 mV
    else if(config.pga == 0){max_range   = 7626    ;}  // get read in range from 0 to 6144 mV
}

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    
    //critical zone start
    critical_section_enter_blocking(&myLock);
    
    //From data sheet of ADS1115 p[35] in section [10.1.7] we obtain the sequence of operation for master bus
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data  
        // writes always start with the memory address
        add_ptr_reg = i2c_read_byte_raw(i2c); 
        // save configration into configration register 
        if(add_ptr_reg == 1){
            uint8_t buffer[2];       //array to receive data from master 
            //receving data from master MSB then LSB
            i2c_read_blocking(i2c, I2C_SLAVE_ADDRESS, buffer, 2, false);
            msb = buffer[0];
            lsb = buffer[1];
            set_configration = (msb << 8) | lsb;                    //set bits in right position
            memcpy(&config, &set_configration, sizeof(uint16_t));  //copy configration in bitfield
        }  
        if(add_ptr_reg == 2){
                i2c_read_blocking(i2c, I2C_SLAVE_ADDRESS, &lo_thresh_reg,2, false);
            }  
        if(add_ptr_reg == 3){
                i2c_read_blocking(i2c, I2C_SLAVE_ADDRESS, &ho_thresh_reg,2, false);
            }         
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data load from ADC
        //Sending data MSB then LSB
        if(add_ptr_reg == 1){
            i2c_write_byte_raw(i2c, (set_configration >> 8) );
            i2c_write_byte_raw(i2c, set_configration);
        }
        else if (add_ptr_reg == 2) {
            i2c_write_byte_raw(i2c, (lo_thresh_reg >> 8) );
            i2c_write_byte_raw(i2c, lo_thresh_reg);
        }
        else if (add_ptr_reg == 3) {
            i2c_write_byte_raw(i2c, (ho_thresh_reg >> 8) );
            i2c_write_byte_raw(i2c, ho_thresh_reg);
        }
        else  {
            adc_settings();
            get_adc_read();        
            i2c_write_byte_raw(i2c, (response >> 8) );
            i2c_write_byte_raw(i2c, response);
        }break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
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
    // configure I2C1 for slave mode
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

    //operating function 
    setup_slave();
    
    while (true);
    
}