#include <pico/time.h>
#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define GPIO_SCK 0
#define GPIO_DATA 1

int main ()
{
    uint i =0;
    bool data_ready = 0;
    uint hx711_out = 0 , hx711_result;
    //initialize GPIO
    gpio_init(GPIO_SCK);
    gpio_set_dir(GPIO_SCK, GPIO_OUT);
    gpio_init(GPIO_DATA);
    gpio_set_dir(GPIO_DATA, GPIO_IN);
    
    while(true)
    {    

        //if DATA PIN IS LOW
        if(data_ready == false)
        {
            sleep_us(10);
            // to extract data 
            for(uint i = 24 ; i >= 1 ; i--)
            {
                gpio_put(GPIO_SCK,true);
                sleep_us(10);
                //To read data  
                hx711_out = gpio_get(GPIO_DATA);
                hx711_result  =  hx711_result  | hx711_out  << (i-1)  ;
                gpio_put(GPIO_SCK,false);
                sleep_us(10);
            }
        }
        //to print signal
        printf("weight bits = %u\n",hx711_result);

    }
    
}