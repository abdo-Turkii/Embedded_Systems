#ifndef ADS_H
#define ADS_H

// For this program, i try to  emulate ADS1115 by using RP2040.
// from RP2040 i using GPIO(26,27,28,29) as a ADC converter with 12 bits resolution. 
// ADS1115 have a 16 bits resolution so we convert 12 bits to 16 bits to send data in 16 bits 
// Datasheet of ADS1115 [https://www.ti.com/lit/ds/symlink/ads1115.pdf].
// Git hub link [https://github.com/abdo-Turkii/Embedded_Systems/blob/main/ads1115_emulation/ads1115.c]
// From data sheet of ADS115 p[28] in section [9.6.3] Config Register Field Descriptions 
// Table(8) Config Register Field Descriptions.
static struct config_register {
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

// You'll need to wire pin GPIO 20 (SDA), and pin GPIO 21 (SCL).
#define I2C0_SLAVE_SDA_PIN 20         // I2C0 bus
#define I2C0_SLAVE_SCL_PIN 21         // I2C0 bus

//Number of register in ADS1115
#ifndef NUM_ADS1115_REGISTERS
#define NUM_ADS1115_REGISTERS 4
#endif
// From data sheet of ADS115 p[27] in section [9.6] Register Map for ADS1115  
// Table (6). Address Pointer Register Field Descriptions
/*Register address pointer
    00 : Conversion register
    01 : Config register
    10 : Lo_thresh register
    11 : Hi_thresh register
*/
// Table (6). Address Pointer Register Field Descriptions
static uint8_t register_address_pointer ;             //register address pointer
static uint16_t register_ads1115[NUM_ADS1115_REGISTERS]={0x0000,0x8483,0x8000,0x7FFF};
//Max range in operation range of ADC
static uint16_t max_range ;                      // max range for input signal

//define critical section lock
static critical_section_t my_critical_section;


#endif