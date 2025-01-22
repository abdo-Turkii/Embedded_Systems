#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <hardware/i2c.h>
#include <pico/critical_section.h>
#include <pico/i2c_slave.h>
#include <pico/runtime_init.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hardware/pwm.h"

// For this program, i try to  emulate PCA9685 by using RP2040.
// from RP2040 i using GPIO (0->15) as a PWM channels with 12 bits resolution. 
// PWM have a 16 bits resolution so we convert 16 bits to 12 bits to send data in 12 bits 
// Feature of LED_ON_H[4] & LE0_OFF_H[4] in PCA9685 is not supported in this program.
// Datasheet of PCA9685 [https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf].
// Git hub link [https://github.com/abdo-Turkii/Embedded_Systems/blob/main/ads1115_emulation/ads1115.c]

// From data sheet of PCA9685 p[14] in section [ 7.3.1 Mode register 1, MODE1]. 
// Table (5). MODE1 - Mode register 1 (address 00h) bit description.
struct register_mode_1 {
    unsigned int allcall : 1;           // Responds to LED All Call I2C-bus address.      
    unsigned int sub3 :    1;          // Responds to I2C-bus subaddress 3.
    unsigned int sub2 :    1;         // Responds to I2C-bus subaddress 2.
    unsigned int sub1:     1;        // Responds to I2C-bus subaddress 1.
    unsigned int sleep:    1;       // Low power mode. Oscillator off.
    unsigned int ai:       1;      // Register Auto-Increment.
    unsigned int extclk:   1;     // Use external clock pin clock or not.
    unsigned int restart:  1;    //These bit to Restart device.
} mode_1;
// From data sheet of PCA9685 p[16] in section [7.3.2 Mode register 2, MODE2]. 
// Table 6. MODE2 - Mode register 2 (address 01h) bit description.
struct register_mode_2 {
    unsigned int outne   :   2;           // output according to output enable pin (OE).      
    unsigned int outdrv  :   1;          // The 16 LEDn outputs are configured totem pole structure vs open-drain .
    unsigned int och     :   1;         // Outputs change on ACK vs STOP Condition.
    unsigned int invert  :   1;        // Output logic state inverted .
    unsigned int reserved:   3;       // reserved.
} mode_2;

//Variables to store the value of PWM 
// From data sheet of PCA9685 p[21] in section [7.3.3 LED output and brightness control].
// Table 7. LED_ON, LED_OFF control registers (address 06h to 45h) bit description
struct led_on_off {
    unsigned int led_on_l   :   8;           // LED output and brightness control byte 0.      
    unsigned int led_on_h   :   8;          // LED output and brightness control byte 1.
    unsigned int led_off_l  :   8;         // LED output and brightness control byte 2.
    unsigned int led_off_h  :   8;        // LED output and brightness control byte 3.
} led;

// I2C address and pins
// Address of i2c slave in RP2040
#define I2C_SLAVE_ADDRESS  0x055    
// 400 kHz for i2c bus data rate transfer    
#define I2C_BAUDRATE       400*1000    

// You'll need to wire pin GPIO 18 (SDA), and pin GPIO 19 (SCL).
#define I2C_SLAVE_SDA_PIN 18            // I2C1 bus 
#define I2C_SLAVE_SCL_PIN 19           //  I2C1 bus                  

//Number of register in PCA9685
#define NUM_REGISTERS 256 

// 12 bit max value for wrap value
#define WRAPVAL 4096   // 12 bit max

// Register summary
// Value of address register in PCA9685
static uint8_t register_address ;  
// Array of registers in PCA9685
static uint8_t register_pca9685[NUM_REGISTERS];
// Flag to check if address of register has written
static bool register_pca9685_address_written = false;
//define critical section lock
critical_section_t myLock;

//Set default value of registers        
void init_pca9685()
{
    register_pca9685[0]    = 0x88; //MODE1
    register_pca9685[1]    = 0x20; //MODE2
    register_pca9685[2]    = 0xE2; //Subaddress1
    register_pca9685[3]    = 0xE4; //Subaddress2
    register_pca9685[4]    = 0xE8; //Subaddress3
    register_pca9685[5]    = 0xE0; //ALL CALL ADDRESS
    register_pca9685[0xFE] = 0xC8; //Pre-scale value 
} 

// Function to configuration GPIO as PWM 
void pwm_configuration(uint8_t channel_number , uint16_t led_on, uint16_t led_off)
{

    // Function to initialize PWM GPIO
    gpio_set_function(channel_number, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 
    uint slice_num = pwm_gpio_to_slice_num(channel_number);

    // Put frequency of PWM
    pwm_set_clkdiv(slice_num,register_pca9685[0xFE]); //divider range is 0-255

    //put start delay of signal
    pwm_set_counter(slice_num, led_on*5);
    
    // Set period cycles
    pwm_set_wrap(slice_num, WRAPVAL*5);
    
    // Choose PWM channel
    if (channel_number % 2) {
        // put time to turn off signal 
        pwm_set_chan_level(slice_num, PWM_CHAN_B, led_off*5);
    }
    else 
        // put time to turn off signal 
        pwm_set_chan_level(slice_num, PWM_CHAN_A, led_off*5);
    // Enable PWM
    pwm_set_enabled(slice_num, true);
}

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    //critical zone start
    critical_section_enter_blocking(&myLock);
    uint8_t i , channel_number ;
    uint16_t led_on, led_off;
    //From data sheet of PCA9685 p[32] in section [9.Bus transactions] we obtain the sequence of operation for master bus
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data  
        if (register_pca9685_address_written == false) {
            //address of register has be written
            register_pca9685_address_written = true; 
            // writes always start with the memory address
            register_address = i2c_read_byte_raw(i2c); 
        }
        else {
            // Save Data into PCA9685 registers 
            register_pca9685[register_address] = i2c_read_byte_raw(i2c);            
            if(register_address == 0)
                //copy bits in bitfield struct
                memcpy(&mode_1, &register_pca9685[register_address], sizeof(uint8_t)); 
            if(register_address == 1)
                //copy bits in bitfield struct
                memcpy(&mode_2, &register_pca9685[register_address], sizeof(uint8_t));   
            if ((register_address > 8) && (register_address < 70) && ((register_address-9)%4 == 0)){ 
                if (mode_1.ai == 0) 
                    register_pca9685_address_written = false;
                channel_number = ((register_address-9)/4);
                // Get value of PWM from registers
                led.led_on_l = register_pca9685[register_address-3];
                led.led_on_h = register_pca9685[register_address-2] & 0x0F;
                led_on = (led.led_on_h << 8) | led.led_on_l;
                led.led_off_l = register_pca9685[register_address-1];
                led.led_off_h = register_pca9685[register_address] & 0x0F;
                led_off = (led.led_off_h << 8) | led.led_off_l;
                if(register_address != 253)
                    // put setting of PWM
                    pwm_configuration(channel_number , led_on, led_off);
                else {
                    for(i = 0; i < 16; i++)
                        // put setting of PWM
                        pwm_configuration(i , led_on, led_off);
                }
            }
            if(register_address < 6)
                //address of register has written
                register_pca9685_address_written = false;
            register_address++; // increment the register address
        }

        break;
    case I2C_SLAVE_REQUEST: // master is requesting data load from ADC
        //address of register has written
        register_pca9685_address_written = false;
        if(mode_1.ai == 1){
            mode_1.ai = 0;
            //auto increment mode
            for(uint8_t i = 0; i < 70; i++)
                    i2c_write_byte_raw(i2c, (register_pca9685[register_address + i])  & 0xFF);
        }
        else       
            // send the DATA to the master
            i2c_write_byte_raw(i2c, (register_pca9685[register_address]) & 0xFF);
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        break;
    default:
        break;
    }
    //critical zone end
    critical_section_exit(&myLock);
}

// Initialize I2C1
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


int main() {
    
    stdio_init_all();
    
    //Initialisation of PCA9685 
    init_pca9685();

    //Initialisation of critical section
    critical_section_init(&myLock);

    //set interrput priority
    irq_set_priority(I2C1_IRQ,0x00);   //Highest priority of irq

    //operating function 
    setup_slave();
    
    while (true);
    
}