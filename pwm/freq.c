#include "pico/stdlib.h"
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include "hardware/pwm.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

uint32_t start, end1 , end2;
static double time1 = 0.00 , time2=0.00 ;
static bool s = true;

void callback1(uint gpio, uint32_t events)
{   
    if (events & GPIO_IRQ_EDGE_FALL) {

        if(s == true)
        {
            start = time_us_32();
            s = false;
        }
        else
        {
            end1 = time_us_32();
            time1 = (end1 - start) / 1000000.0;
            s = true;
        }
    }
    
    if (events & GPIO_IRQ_EDGE_RISE && s == true) {    
        end2 = time_us_32();
        time2 = (end2 - start) / 1000000.0;
    }
             
}

int main() {
    // Initialize stdio
    stdio_init_all();

    // Set the GPIO pin for PWM input
    uint pin = 3; // Example GPIO pin
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);

    gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE  , true, &callback1);

    while (true)
    {
        if(s)
        {
            printf("Freq: %.02lf Hz\n", (1/time1));
            printf("Duty cycle: %.02lf %% \n", (((1-(time2/time1))*100))+100);
        }
        sleep_ms(1000);
    }
    
    
}
