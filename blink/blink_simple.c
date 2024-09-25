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

void turn_on()
{
    bool on ;
    on = gpio_get(24);
    gpio_put(25,!on);
    printf("LED IS ON = %d\n",!on);
}

/*void turn_off()
{
    bool on = 0 ;
    //on = gpio_get(24);
    gpio_put(25,on);
    printf("LED IS OFF = %d\n",on);
}*/

int main() 
{  
    stdio_init_all();

    gpio_init(25);
    gpio_init(24);
    gpio_set_dir(25,   1);
    gpio_set_dir(24, 0);
    gpio_pull_up(24);
    //gpio_put(25,1);
    //sleep_ms(2000);
    printf("Start prog LED IS OFF\n");

    //interrupt function
    gpio_set_irq_enabled_with_callback(24, GPIO_IRQ_EDGE_FALL ,true,&turn_on);

    gpio_set_irq_enabled_with_callback(24, GPIO_IRQ_EDGE_RISE ,true,&turn_on);


    while(true)
    {
        printf("Sleeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeping\n");
        sleep_ms(100);

    }

}
