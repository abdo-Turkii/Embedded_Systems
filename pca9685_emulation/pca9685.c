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
// from RP2040 i using GPIO(26,27,28,29) as a PWM with 12 bits resolution. 
// PWM have a 16 bits resolution so we convert 16 bits to 12 bits to send data in 12 bits 
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
struct led_on_off {
    unsigned int led_on_l   :   8;           // LED output and brightness control byte 0.      
    unsigned int led_on_h   :   8;          // LED output and brightness control byte 1.
    unsigned int led_off_l  :   8;         // LED output and brightness control byte 2.
    unsigned int led_off_h  :   8;        // LED output and brightness control byte 3.
} led;
// I2C address and pins
#define I2C_SLAVE_ADDRESS  0x055        // Address of i2c slave in RP2040
#define I2C_BAUDRATE       400*1000    // 400 kHz for i2c bus data rate transfer

// You'll need to wire pin GPIO 16 (SDA), and pin GPIO 17 (SCL).
#define I2C_SLAVE_SDA_PIN 18
#define I2C_SLAVE_SCL_PIN 19

//Number of register in PCA9685
#define NUM_REGISTERS 256 

//
#define WRAPVAL 4096   // 12 bit max

//Register summary
static uint8_t register_address,j=0 ;             //register address
static uint8_t register_pca9685[NUM_REGISTERS];
static bool register_pca9685_address_written = true;
//define critical section lock
critical_section_t myLock;
        
    
void pwm_configuration(uint8_t channel_number , uint16_t led_on, uint16_t led_off)
{
    printf("Channel number is: %d\n",channel_number);

    // Function to initialize PWM GPIO
    gpio_set_function(channel_number, GPIO_FUNC_PWM);
    // Find out which PWM slice is connected to GPIO 
    uint slice_num = pwm_gpio_to_slice_num(channel_number);
    // Put frequency of PWM
    //pwm_set_clkdiv(slice_num,5 );
    pwm_set_clkdiv(slice_num,200);
    //put start delay of signal
    pwm_set_counter(slice_num, led_on);
    
    // Set period cycles
    pwm_set_wrap(slice_num, WRAPVAL);
    
    // put time to turn off signal 
    pwm_set_chan_level(slice_num, PWM_CHAN_B, led_off);
    // Enable PWM
    pwm_set_enabled(slice_num, true);
}

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    //critical zone start
    critical_section_enter_blocking(&myLock);
    uint8_t i ,channel_number ;
    uint16_t led_on, led_off;
    //From data sheet of PCA9685 p[32] in section [9.Bus transactions] we obtain the sequence of operation for master bus
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data  
    if (register_pca9685_address_written == true) {
        // writes always start with the memory address
        register_address = i2c_read_byte_raw(i2c); 
        //printf("%d-Register address is: %d\n",j,register_address);
        j=j+1;
        register_pca9685_address_written = false;
        // Save Data into PCA9685 registers 
        //register_pca9685[register_address] = i2c_read_byte_raw(i2c); 
        //printf("register_pca9685[register_address]: %d\n",register_pca9685[register_address]);
    }
    else
        {  
            register_pca9685_address_written = false;
            for( i = 1; i < 76; i++)
            register_pca9685[register_address] = i2c_read_byte_raw(i2c); 
            printf("%d-Register address is: %d,register_pca9685[register_address]: %d\n",j,register_address,register_pca9685[register_address]);
            j=j+1;
            /*if(register_address == 0)
                //copy bits in bitfield struct
                memcpy(&mode_1, &register_pca9685[register_address], sizeof(uint8_t)); 
            if(mode_1.ai == 1){
                mode_1.ai = 0;
                //auto increment mode
                for( i = 1; i < 70; i++){
                    register_pca9685[register_address + i] = i2c_read_byte_raw(i2c); 
                    if ((i > 8) && ((i-9)%4 == 0)){         
                        channel_number = ((i-9)/4);
                        //copy bits in bitfield struct
                        led.led_on_l = register_pca9685[i-3];
                        led.led_on_h = register_pca9685[i-2] & 0x0F;
                        led_on = (led.led_on_h << 8) | led.led_on_l;
                        led.led_off_l = register_pca9685[i-1];
                        led.led_off_h = register_pca9685[i] & 0x0F;
                        led_off = (led.led_off_h << 8) | led.led_off_l;
                        //i=i+4;  
                        // put setting of PWM
                        pwm_configuration(channel_number , led_on, led_off);
                    }
                }
            }
        }
        if(register_address == 1)
            //copy bits in bitfield struct
            memcpy(&mode_2, &register_pca9685[register_address], sizeof(uint8_t));   
        */
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data load from ADC
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
        register_pca9685_address_written = false;
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


int main() {
    
    stdio_init_all();
    
    //clocks_init();
    //clock_configure(clk_ref, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, 125000000, 25000000); // 25 MHz
    
    // Default value of register
    uint prescale = 0xC8; // prescaler to program the PWM output frequency (default is 200 Hz).

    //initialisation PWM channels 
    //for(uint8_t i = 0; i < 16; i++)
        //pwm_configuration(i, 0, 4096/4);
    
    //initialisation of critical section
    critical_section_init(&myLock);

    //set interrput priority
    irq_set_priority(I2C1_IRQ,0x00);   //Highest priority of irq

    //operating function 
    setup_slave();
    
    while (true);
    /*{
        double x = clock_get_hz(clk_sys);
        printf("Clock sys is: %.02lf\n",x);
        x = clock_get_hz(clk_ref);
        printf("Clock ref is: %.02lf\n",x);
        sleep_ms(1000);
    }*/
    
}