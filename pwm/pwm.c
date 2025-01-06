/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1

#include "pico/stdlib.h"
#include <pico/stdio.h>
#include <pico/time.h>
#include "hardware/pwm.h"
#include <stdio.h>

#define WRAPVAL 4096   //16 bit max
#define CLKDIV 50.0f
#define CPU_CLK 125000000.0f

int main() {
    stdio_init_all();
    /// \tag::setup_pwm[]

    // Tell GPIO 25 they are allocated to the PWM
    gpio_set_function(1, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 25 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(1);

	pwm_set_clkdiv(slice_num, CLKDIV);

    // Set period of 4 micro second (500) cycles 
    pwm_set_wrap(slice_num, WRAPVAL);
	// 50% duty time 
	pwm_set_chan_level(slice_num, PWM_CHAN_B, WRAPVAL/(10));
	 
	pwm_set_enabled(slice_num, true);
   
    while(true);    
    //{
        //for(int i=1 ;i<=500;i++)
        //{
			//printf("The PWM freq  = %lf\n", (CPU_CLK/CLKDIV)/WRAPVAL);
			//sleep_ms(1000);
		//}    
    //}
   
}