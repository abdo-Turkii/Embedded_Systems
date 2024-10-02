#include <hardware/timer.h>
#include <pico/platform/compiler.h>
#include <pico/types.h>
#include <stdio.h>
#include <time.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include <string.h>
#include <stdlib.h>
#include "hardware/dma.h"


#define MAX_ARR 100
//#define DATA_CHAN 0
//#define CTRL_CHAN 1


int main()
{
    stdio_init_all();

    uint i ;
    int arr_a[MAX_ARR] ,arr_b[MAX_ARR],arr_c[MAX_ARR],arr_d[MAX_ARR];
    uint32_t start, end; 
    double time_for , time_dma , time_memcpy;
    //arr_a[]={0,1,2,3,4,.......,99}
    for(i = 0; i < MAX_ARR; i++ )
    {
        arr_a[i] = i;
    }
        
    //to copy arr_a in arr_b by For loop
    start = time_us_64();
    for(i = 0; i < MAX_ARR; i++ )
    {
        arr_b[i] = arr_a[i] ;
    }
    end = time_us_64();
    time_for = end - start ;
       
    // Get a free channel, panic() if there are none
    int chan = dma_claim_unused_channel(true);

    // 8 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    // No DREQ is selected, so the DMA transfers as fast as it can.

    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);
    
    start = time_us_64();

    dma_channel_configure(
        chan,                   // Channel to be configured
        &c,                     // The configuration we just created
        &arr_c,            // The initial write address
        &arr_a,            // The initial read address
        100,         // Number of transfers; in this case each is 1 byte.
        true               // Start immediately.
    );

    // We could choose to go and do something else whilst the DMA is doing its
    // thing. In this case the processor has nothing else to do, so we just
    // wait for the DMA to finish.
    dma_channel_wait_for_finish_blocking(chan);
    end = time_us_64();
    time_dma = end - start ;

    
    //to copy arr_a in arr_b by memcpy() function
    start = time_us_64();
    memcpy(arr_d, arr_a, sizeof(arr_a));
    end = time_us_64();
    time_memcpy = end - start ;
    
    
    while(true)
    {
        printf("==========================================================\n");
        printf("Time Consumed by for loop = %lf \n",time_for );
        printf("Time Consumed by master buse DMA  = %lf \n",time_dma);
        printf("Time Consumed by  memcpy() function = %lf \n",time_memcpy );
        printf("==========================================================\n");
        sleep_ms(1000);

    }
}