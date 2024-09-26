#include <pico/stdio.h>
//#include "pico/binary_info.h"
#include <pico/time.h>
#include <stdio.h>
#include <time.h>

#define LOOP (1<<20)

int main()
{
	stdio_init_all();
	unsigned int i  ;
	time_us_32() start ,end;
	int a=10,b=5,c=0;
	float   x=5.3, y=4.7 , z=0.0;
	double  s=10.11,k=2.99,d=0.0;

	time_us_32() start = clock();
	for( i = 0 ; i < LOOP ; i++)
    {
		c = i  + b ;
	}
	clock_t end = clock();
	double seconds_int = (double)(end - start) / CLOCKS_PER_SEC ;

	start = clock();
	for( i = 0 ; i < LOOP ; i++)
    {
		z = i + y ;
	}
	end = clock();
	double seconds_fl = (double)(end - start) / CLOCKS_PER_SEC ;
	

	start = clock();
	for( i = 0 ; i < LOOP ; i++)
    {
		d = i + k ;
	}
	end = clock();
	double seconds_double = (double)(end - start) / CLOCKS_PER_SEC ;

	while (true)
	{
		printf("\nTime for int %lf\n",seconds_int);
		printf("\nTime for float %lf\n",seconds_fl);
		printf("\nTime for Double %lf\n",seconds_double);

		sleep_ms(1000);
	}	
}	
