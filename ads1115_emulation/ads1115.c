#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/critical_section.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"
// For this program, we emulat ADS1115 by using RP2040.

// from section [9.6.3]  Config Register Field Descriptions 
//table Config Register Field Descriptions
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

static uint16_t set_configration =0x0483  , input_signal , response  ;
static uint8_t add_ptr_reg = 0 , msb , lsb ;   // reg_add refar to the number of register
uint16_t max_range ;                      // max range for input signal
static bool ack ;                        // acknowledge bit
// I2C address and pins
static const uint I2C_SLAVE_ADDRESS = 0x055;   //address of i2c slave in rp2040
static const uint I2C_BAUDRATE = 100*1000;    // 100 kHz 

//define critical section lock
critical_section_t myLock;

#ifdef i2c1
// You'll need to wire pin GPIO 18 (SDA), and pin GPIO 19 (SCL).
static const uint I2C_SLAVE_SDA_PIN = 18; 
static const uint I2C_SLAVE_SCL_PIN = 19; 

//Function to get signal and convert it to 15 bits.
void get_adc_read()
{
    input_signal = adc_read();           //get signal from ADC after convertion it to digital
    if(input_signal >= max_range)       //saturate signal 
        response = 32767;                //MAX in range 
    else
        response = (input_signal * 32767 ) / max_range ;   //Store signal in 15 bits 
}

//Function to initialize ADC GPIO
void adc_config()
{
    // Initialise the ADC. reset clock for ABC 
    adc_init();                                 
    adc_gpio_init(ADC_26);                   // GPIO 26 use as ADC pin
    adc_gpio_init(ADC_27);                  // GPIO 27 use as ADC pin
    adc_gpio_init(ADC_28);                 // GPIO 28 use as ADC pin
    adc_gpio_init(ADC_29);                // GPIO 29 use as ADC pin  
}

//Function to apply configration 
void adc_settings()
{
    //Select adc pin input from  pin 0 to 3 (Gpio from 26 to 29)
    adc_select_input(config.mux % 4); 

    //from section [9.6.3] Config Register
    //Seleect PGA from FSR = 0.256 to 6.144 V
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
    
    //From section [10.1.7] we obtain the sequence of operation for master bus
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
            if(!ack){
            // writes always start with the memory address
            add_ptr_reg = i2c_read_byte_raw(i2c); 
            ack = true ;
            }
            // save configration  into configration register 
            else {
                //add_ptr_reg = 0 ;
                i2c_read_blocking(i2c, I2C_SLAVE_ADDRESS, &set_configration,2, false);
                msb = set_configration ;
                lsb = (set_configration >> 8) ;
                set_configration = (msb << 8) | lsb;
                memcpy(&config,&set_configration,sizeof(uint16_t));
            }
            
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data load from ADC
        printf("Most[%u]-Least[%u]\n",msb,lsb);
        adc_settings();
        get_adc_read();
        printf("pin[%u]-pga[%u]\n",(config.mux%4),config.pga);
        i2c_write_byte_raw(i2c, (response >> 8) );
        i2c_write_byte_raw(i2c, response);
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        ack = false ;
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