#include <pico/time.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define WRAPVAL 65535 // 16 bit max


int main()
{
    stdio_init_all();
    
    uint single_adc0=0,single_adc1=0,single_adc2=0,single_adc3=0;
    double result0 ,result1,result2,result3 ;

    gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);

    adc_init();                                   // Initialise the ADC. reset clock for ABC 
    adc_gpio_init(26);                    // GPIO use as ADC pin
    adc_gpio_init(27);                  // GPIO use as ADC pin
    adc_gpio_init(28);                 // GPIO use as ADC pin
    adc_gpio_init(29);
    
	 
    while (true) {
        int adc_chan = 0; 
        adc_select_input(adc_chan); 
        single_adc0 = adc_read();                       //Performs an ADC conversion
        
        pwm_set_wrap(slice_num, (single_adc0)*16);
        pwm_set_clkdiv(slice_num, 256.0);
        pwm_set_chan_level(slice_num, PWM_CHAN_B, single_adc0*8);
    	pwm_set_enabled(slice_num, true);

        printf("VOLT_ADC0 = %u\n",single_adc0);
        /*adc_chan++;

        adc_select_input(adc_chan); 
        single_adc1 = adc_read();                       //Performs an ADC conversion
        adc_chan++;

        adc_select_input(adc_chan); 
        single_adc2 = adc_read();                       //Performs an ADC conversion
        adc_chan++;

        adc_select_input(adc_chan); 
        single_adc3 = adc_read();   
        */
       // result0 = (single_adc0 * 3.3 )/4095 ;            // conversion
        //result1 = (single_adc1 * 3.3 )/4095 ;            // conversion
        //result2 = (single_adc2 * 3.3 )/4095 ;            // conversion
        //result3 = (single_adc3 * 3.3 )/4095 ;            // conversion

        //printf("VOLT_ADC0 = %.02lf\t VOLT_ADC1 = %.02lf\t VOLT_ADC2 = %.02lf\t VOLT_ADC2 = %.02lf\n",result0,result1,result2,result3);            //print results
        
        //printf("VOLT_ADC0 = %u\t VOLT_ADC1 = %u\t VOLT_ADC2 = %u\t VOLT_ADC3 = %u\n",single_adc0,single_adc1,single_adc2,single_adc3);            //print results
        sleep_ms(500);

    }
}