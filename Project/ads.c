#include "definition.h"

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
    // variable for signal readd by GPIO of RP2040  
    uint16_t input_signal  ;                        
    
    //get signal from ADC after conversion it to digital signal
    input_signal = adc_read();                           
    //Saturation of signal
    if(input_signal >= max_range)                       
        //Max in range .
        register_ads1115[register_address_pointer] = 32767;                   
    else
        //Store signal in 15 bits in his range
        register_ads1115[register_address_pointer] = (input_signal * 32767 ) / max_range ;   
}

//Function to apply configuration 
void adc_settings()
{
    //Select adc pin input from pin 0 to 3 (Gpio from 26 to 29)
    adc_select_input(config.mux % 4); 

    //From datas sheet of ADS1115 p[17] in section [9.3.3] Full-Scale Range (FSR) and LSB Size
    //Table (3). Full-Scale Range and Corresponding LSB Size
    //Select PGA from [FSR = 0.256 to 6.144]V
    switch (config.pga) {
    case 0:
        max_range   = 7626    ;    // get read in range from 0 to 6144 mV
        break;
    case 1:
        max_range   = 5084    ;    // get read in range from 0 to 4096 mV
        break;
    case 2:
        max_range   = 2542    ;    // get read in range from 0 to 2048 mV
        break;
    case 3:
        max_range   = 1271    ;    // get read in range from 0 to 1024 mV
        break;
    case 4:
        max_range   = 635     ;    // get read in range from 0 to 512  mV
        break;
    case 5:
        max_range   = 328     ;    // get read in range from 0 to 256  mV
        break;
    default:
        max_range   = 2542    ;    // get read in range from 0 to 2048 mV
        break;
    }
}

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c0_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    //critical zone start
    critical_section_enter_blocking(&my_critical_section);
    
    //array to receive data from master 
    uint8_t buffer[2];                     
    // 2 bytes of data come from master i2c bus [default configuration]
    // MSB refer to the most significant byte and LSB refer to the least significant byte
    uint8_t msb = 0x84 , lsb = 0x83 ;    
    //From data sheet of ADS1115 p[35] in section [10.1.7] we obtain the sequence of operation for master bus
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data  
        // writes always start with the memory address
        register_address_pointer = i2c_read_byte_raw(i2c); 
        if(register_address_pointer != 0){
            // save configuration into configuration register 
            i2c_read_blocking(i2c, I2C_SLAVE_ADDRESS, buffer, 2, false);
                // receive the most significant byte from the master
                msb = buffer[0];
                // receive the least significant byte from the master
                lsb = buffer[1] ;
                register_ads1115[register_address_pointer] = (msb << 8) | lsb;
                if(register_address_pointer == 1)
                    //copy configuration in bitfield struct
                    memcpy(&config, &register_ads1115[register_address_pointer], sizeof(uint16_t));           
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data load from ADC
        if(register_address_pointer == 0){
            //adjust ADC settings
            adc_settings();
            //get signal from GPIO
            get_adc_read();        
        }
        // send the MSB to the master
        i2c_write_byte_raw(i2c, (register_ads1115[register_address_pointer] >> 8) & 0xFF);
        // send the LSB to the master
        i2c_write_byte_raw(i2c, register_ads1115[register_address_pointer] & 0xFF);
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        break;
    default:
        break;
    }
    //critical zone end
    critical_section_exit(&my_critical_section);
}

void setup_i2c0_slave() {
    gpio_init(I2C0_SLAVE_SDA_PIN);
    gpio_set_function(I2C0_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SLAVE_SDA_PIN);

    gpio_init(I2C0_SLAVE_SCL_PIN);
    gpio_set_function(I2C0_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    // configure I2C1 for slave mode
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c0_slave_handler);
}
