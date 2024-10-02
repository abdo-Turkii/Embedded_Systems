#include <pico/stdio.h>
#include <pico/time.h>
#include <stdio.h>
#include <time.h>

#define LOOP (1<<20)

int main()
{
  stdio_init_all();

  uint32_t start , end;  // variable to store time
  //variable to measure exact time spend for operation 
  double seconds_int, seconds_fl, seconds_double, seconds_char,seconds_bool; 
  int     a = 15 ,b = 55 ,c = 0 ,i=0;
	float   x=5.3, y=4.7 , z=0.0;
	double  s=10.11,k=2.99,d=0.0;
  char    q = 'q' , w = 'w' ,e ;
  bool    h = 0 , o =1 , j = 0 ; 

  start=time_us_64(); // variable to store when operation start
  //for( i = 0 ; i < LOOP ; i++) //loop for amplifier time by 2 power 19
  //{
		c = a  + b ; //sum operation
	//}    
  end=time_us_64(); // variable to store when operation end
  seconds_int = end - start; // to calculate time 
    
    //repeated
  //for( i = 0 ; i < LOOP ; i++)
//{
		z = x  + y ;
//}    
  end=time_us_64();
  seconds_fl = end - start;

  //for( i = 0 ; i < LOOP ; i++)
  //{
		d = k  + s ;
	//}    
  end=time_us_64();
  seconds_double = end - start;

  for( i = 0 ; i < LOOP ; i++)
  {
		e = q  + w ;
	}    
  end=time_us_64();
  seconds_char = end - start;

  for( i = 0 ; i < LOOP ; i++)
  {
		j = h  + o ;
	}    
  end=time_us_64();
  seconds_bool = end - start;

//to print time of operation
  while ( true )
  {

		printf("\nTime for int %lf\n",seconds_int);
		printf("\nTime for float %lf\n",seconds_fl);
		printf("\nTime for Double %lf\n",seconds_double);
		printf("\nTime for char %lf\n",seconds_char);
		printf("\nTime for Bool %lf\n",seconds_bool);
		sleep_ms(5000);
	}	
       

}