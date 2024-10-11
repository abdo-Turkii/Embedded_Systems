#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define GPIO_SCK 16
#define GPIO_DATA 17
#define OFFSET (66600+66800)/2

int main ()
{
    stdio_init_all();

    char c;
    int i  ;
    bool hx711_out = 0 ;
    int  hx711_result;
    double Weight_g;
    //initialize GPIO
    gpio_init(GPIO_SCK);
    gpio_set_dir(GPIO_SCK, GPIO_OUT); //Gpio 0 is output 
    gpio_pull_down(GPIO_SCK);
    gpio_init(GPIO_DATA);
    gpio_set_dir(GPIO_DATA, GPIO_IN); //Gpio 1 is input 
    gpio_pull_up(GPIO_DATA);
    //scanf("%c",&c);
    //hx711_out = gpio_get(GPIO_DATA);  // get one clock 
    //printf("DAaaaaaaaaTA READY = %d \n",hx711_out);
    //reset HX711
    gpio_put(GPIO_SCK,true); //high clock 
    sleep_us(100);
    gpio_put(GPIO_SCK,false); //low clock 

    while(true)
    {    
        hx711_result = 0;
        hx711_out = gpio_get(GPIO_DATA);  // get one clock 
        //printf("DATA READY = %d \n",hx711_out);

        //if DATA PIN IS LOW
        if(hx711_out == false)  //Check DOUT have Data or not 
        {
            sleep_us(10);      //wait time for first clock
            // to extract data 
            for(i = 24 ; i >= 0 ; i--)
            {
                //printf("Counter of i = %d\n",i);
                gpio_put(GPIO_SCK,true); //rise edge start 
                sleep_us(10);   // peak(high) time 
                //To read data  
                hx711_out = gpio_get(GPIO_DATA);  // get one clock 
                if(i != 0)
                    hx711_result  =  hx711_result  | hx711_out  << (i-1); // set bit
                gpio_put(GPIO_SCK,false); //falling edge start
                sleep_us(10); //low time 
            }
            //hx711_result=(hx711_result-OFFSET);
            Weight_g = ((hx711_result*1000)/(1<<24));
            //to print signal
            printf("The Analog to digital converter  = %u\t Weight = %.02lf gram\n ",hx711_result,Weight_g);

        }

    }
    
} 