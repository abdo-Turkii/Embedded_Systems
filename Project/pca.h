#ifndef PCA_H
#define PCA_H

// For this program, i try to  emulate PCA9685 by using RP2040.
// from RP2040 i using GPIO (0->15) as a PWM channels with 12 bits resolution. 
// PWM have a 16 bits resolution so we convert 16 bits to 12 bits to send data in 12 bits 
// Feature of LED_ON_H[4] & LE0_OFF_H[4] in PCA9685 is not supported in this program.
// Datasheet of PCA9685 [https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf].
// Git hub link [https://github.com/abdo-Turkii/Embedded_Systems/blob/main/ads1115_emulation/ads1115.c]

// From data sheet of PCA9685 p[14] in section [ 7.3.1 Mode register 1, MODE1]. 
// Table (5). MODE1 - Mode register 1 (address 00h) bit description.
static struct register_mode_1 {
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
static struct register_mode_2 {
    unsigned int outne   :   2;           // output according to output enable pin (OE).      
    unsigned int outdrv  :   1;          // The 16 LEDn outputs are configured totem pole structure vs open-drain .
    unsigned int och     :   1;         // Outputs change on ACK vs STOP Condition.
    unsigned int invert  :   1;        // Output logic state inverted .
    unsigned int reserved:   3;       // reserved.
} mode_2;

//Variables to store the value of PWM 
// From data sheet of PCA9685 p[21] in section [7.3.3 LED output and brightness control].
// Table 7. LED_ON, LED_OFF control registers (address 06h to 45h) bit description
static struct led_on_off {
    unsigned int led_on_l   :   8;           // LED output and brightness control byte 0.      
    unsigned int led_on_h   :   8;          // LED output and brightness control byte 1.
    unsigned int led_off_l  :   8;         // LED output and brightness control byte 2.
    unsigned int led_off_h  :   8;        // LED output and brightness control byte 3.
} led;
 
// You'll need to wire pin GPIO 18 (SDA), and pin GPIO 19 (SCL).
#define I2C1_SLAVE_SDA_PIN 18            // I2C1 bus 
#define I2C1_SLAVE_SCL_PIN 19           //  I2C1 bus                  

//Number of register in PCA9685
#ifndef NUM_PCA9685_REGISTERS
#define NUM_PCA9685_REGISTERS 256
#endif

// 12 bit max value for wrap value
#define WRAPVAL 4096   // 12 bit max

// Register summary
// Value of address register in PCA9685
static uint8_t pca9685_register_address ;  
// Array of registers in PCA9685
static uint8_t register_pca9685[NUM_PCA9685_REGISTERS];
// Flag to check if address of register has written
static bool register_pca9685_address_written = false;

#endif