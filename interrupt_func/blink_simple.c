/* 
    This program to explain how interrups occur.
 programe to open led when push button turn on led 
and when push button agine led turn off. 
 by interrupt function because it more save power of cpu.

*/

#include <pico/time.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

bool on = 0  ;

#define PIN_BUTTON 24 
//Function callback when interrupts occur 
void invert() 
{
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    //on = gpio_get(PIN_BUTTON);
    
    gpio_put(PICO_DEFAULT_LED_PIN, on); //to put on value in pin 25
    on = !on ; //to invert on value
    printf("ON = %d \n",on);
}


int main() {
    stdio_init_all(); //to initilise all pins

    //gpio_init(PICO_DEFAULT_LED_PIN);
    //gpio_init(PIN_BUTTON);
    gpio_pull_up(PIN_BUTTON); //to put pin with vss whit open key that connected to ground
    gpio_set_dir(PICO_DEFAULT_LED_PIN,  GPIO_OUT); // make pin output
    gpio_set_dir(PIN_BUTTON,  GPIO_IN);           // make pin input
    gpio_put(PICO_DEFAULT_LED_PIN, true);      //put pin in high voltage
    sleep_ms(5000);  //block for 5 sec

    printf("Hello GPIO IRQ\n");  //message

    //function to make interrupts
    gpio_set_irq_enabled_with_callback(PIN_BUTTON, GPIO_IRQ_EDGE_RISE , true, &invert);

    // Wait forever
    while(true)
    {
        printf("i want to sleep\n");
        sleep_ms(100);
    }
}