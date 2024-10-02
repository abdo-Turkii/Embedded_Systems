/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include <stdio.h>
#define PICO_DEFAULT_BUTTON_PIN 24

void turn_on()
{
    bool on ;
    on = gpio_get(PICO_DEFAULT_BUTTON_PIN);
    gpio_put(PICO_DEFAULT_LED_PIN,!on);
    printf("LED IS ON = %d\n",!on);
}

/*void turn_off()
{
    bool on = 0 ;
    //on = gpio_get(PICO_DEFAULT_BUTTON_PIN);
    gpio_put(PICO_DEFAULT_LED_PIN,on);
    printf("LED IS OFF = %d\n",on);
}*/

int main() 
{  
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_init(PICO_DEFAULT_BUTTON_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN,   1);
    gpio_set_dir(PICO_DEFAULT_BUTTON_PIN, 0);
    gpio_pull_up(PICO_DEFAULT_BUTTON_PIN);
    //gpio_put(PICO_DEFAULT_LED_PIN,1);
    //sleep_ms(2000);
    printf("Start prog LED IS OFF\n");

    //interrupt function
    gpio_set_irq_enabled_with_callback(PICO_DEFAULT_BUTTON_PIN, GPIO_IRQ_EDGE_FALL ,true,&turn_on);

    gpio_set_irq_enabled_with_callback(PICO_DEFAULT_BUTTON_PIN, GPIO_IRQ_EDGE_RISE ,true,&turn_on);


    while(true)
    {
        printf("Sleeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeping\n");
        sleep_ms(100);

    }

}
