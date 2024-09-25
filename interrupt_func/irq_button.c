#include <stdio.h>

#include "pico/stdlib.h"

#define PICO_DEFAULT_BUTTON_PIN 24
//https://forums.raspberrypi.com/viewtopic.php?t=333755

void invert()
{
    bool led_state  =  gpio_get(PICO_DEFAULT_LED_PIN);
    
    gpio_put(PICO_DEFAULT_LED_PIN, !led_state);
    printf("Now we swith the led %d",!led_state);
      

}

int main() {
    stdio_init_all();
    //gpio_init(PICO_DEFAULT_LED_PIN);
    uint bins = (1<<PICO_DEFAULT_LED_PIN) | (1<<PICO_DEFAULT_BUTTON_PIN);
    // Direction ==> zero means input , 1 is output 
    uint bin_direction = (1<<PICO_DEFAULT_LED_PIN) | (0<<PICO_DEFAULT_BUTTON_PIN);
    gpio_init_mask(bins);
    gpio_set_dir_masked(bins,bin_direction);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(5000);

    gpio_pull_up(PICO_DEFAULT_BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(PICO_DEFAULT_BUTTON_PIN,GPIO_IRQ_EDGE_RISE,true,&invert);
    while(true)
    {
        sleep_ms(3000);
    }
}
