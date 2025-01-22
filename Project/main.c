#include "definition.h"

int main() {
    
    stdio_init_all();
    
    //initialization of critical section
    critical_section_init(&my_critical_section);

    //set interrput priority
    irq_set_priority(I2C0_IRQ,0x00);   //Highest priority of irq

    //initialisation ADC channels 
    adc_config();
    
    //Initialisation of PCA9685 
    init_pca9685();
    
    //operating function 
    setup_i2c0_slave();

    //operating function 
    setup_i2c1_slave();
    
    while (true);
    
}

