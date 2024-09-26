#include <hardware/gpio.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"

// event occur when intrrupts occurs
void callback()
{
    static int i = 0;
    static bool on  ;
    if( i == 0)
    {
        on = gpio_get(24);
        i++;
    }
    on = !on;
    gpio_put(25,on);
    printf("ON-OFF ==%d    %d\n",on,i);
    
}
// main finction of core1 
 void core1_fun()
{
    multicore_fifo_clear_irq();
    gpio_set_irq_enabled_with_callback(24,GPIO_IRQ_EDGE_FALL,true,&callback);        

    //irq_set_exclusive_handler(SIO_IRQ_PROC1, callback);
    //irq_set_enabled(SIO_IRQ_PROC1, true);
    /*bool on = multicore_fifo_pop_blocking();
    while(true)
    {
        gpio_put(25,on);
        sleep_ms(100);
        printf("ONnnnnnnnnnnnn\n");
        gpio_put(25,!on);
        sleep_ms(100);
        printf("OFFFFFFFFFFF\n");
        sleep_ms(1000);
    }*/

    while (true)
    {
        printf("OFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n");
        sleep_ms(1000);

    }

}

// main finction of core0
int main()
{
    stdio_init_all();
    //static bool on = 0;

    gpio_init(25);
    gpio_init(24);
    gpio_set_dir(25,1);
    gpio_set_dir(24,0);
    gpio_pull_up(24);

    multicore_reset_core1();
    multicore_launch_core1(core1_fun); // Start core 1 - Do this before any interrupt configuration
    //multicore_fifo_push_blocking(on);

    while (true) 
    {
        printf("core00000000000000000000000000000000000000\n");
        sleep_ms(1000);
    }
}

