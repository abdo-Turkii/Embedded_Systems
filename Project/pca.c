#include "definition.h"

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
static void i2c1_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    // Variable to store the data of channels
    uint8_t  ch , channel_number ;
    uint16_t led_on, led_off;
    //From data sheet of PCA9685 p[32] in section [9.Bus transactions] we obtain the sequence of operation for master bus
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data  
        if (register_pca9685_address_written == false) {
            //address of register has be written
            register_pca9685_address_written = true; 
            // writes always start with the memory address
            pca9685_register_address = i2c_read_byte_raw(i2c); 
        }
        else {
            // Save Data into PCA9685 registers 
            register_pca9685[pca9685_register_address] = i2c_read_byte_raw(i2c);            
            if(pca9685_register_address == 0)
                //copy bits in bitfield struct
                memcpy(&mode_1, &register_pca9685[pca9685_register_address], sizeof(uint8_t)); 
            if(pca9685_register_address == 1)
                //copy bits in bitfield struct
                memcpy(&mode_2, &register_pca9685[pca9685_register_address], sizeof(uint8_t));   
            // set the value of PWM in registers 
            if ((pca9685_register_address > 8) && (pca9685_register_address < 70) && ((pca9685_register_address-9)%4 == 0)){ 
                if (mode_1.ai == 0) 
                    register_pca9685_address_written = false;
                channel_number = ((pca9685_register_address-9)/4);
                // Get value of PWM from registers
                led.led_on_l = register_pca9685[pca9685_register_address-3];
                led.led_on_h = register_pca9685[pca9685_register_address-2] & 0x0F;
                led_on = (led.led_on_h << 8) | led.led_on_l;
                led.led_off_l = register_pca9685[pca9685_register_address-1];
                led.led_off_h = register_pca9685[pca9685_register_address] & 0x0F;
                led_off = (led.led_off_h << 8) | led.led_off_l;
                if(pca9685_register_address != 253)
                    // put setting of PWM on GPIO
                    pwm_configuration(channel_number , led_on, led_off);
                else {
                    for(ch = 0; ch < 16; ch++)
                        // put setting of PWM on GPIO
                        pwm_configuration(ch , led_on, led_off);
                }
            }
            if(pca9685_register_address < 6)
                //address of register has written
                register_pca9685_address_written = false;
            pca9685_register_address++; // increment the register address
        }

        break;
    case I2C_SLAVE_REQUEST: // master is requesting data load from ADC
        //address of register has written
        register_pca9685_address_written = false;
        if(mode_1.ai == 1){
            mode_1.ai = 0;
            //auto increment mode
            for(uint8_t i = 0; i < 70; i++)
                    i2c_write_byte_raw(i2c, (register_pca9685[pca9685_register_address + i])  & 0xFF);
        }
        else       
            // send the DATA to the master
            i2c_write_byte_raw(i2c, (register_pca9685[pca9685_register_address]) & 0xFF);
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        break;
    default:
        break;
    }
}

// Initialize I2C1
 void setup_i2c1_slave() {
    
    gpio_init(I2C1_SLAVE_SDA_PIN);
    gpio_set_function(I2C1_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SLAVE_SDA_PIN);


    gpio_init(I2C1_SLAVE_SCL_PIN);
    gpio_set_function(I2C1_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SLAVE_SCL_PIN);

    i2c_init(i2c1, I2C_BAUDRATE);
    // configure I2C1 for slave mode
    i2c_slave_init(i2c1, I2C_SLAVE_ADDRESS, &i2c1_slave_handler);
}
