#include "keypad.h"
#include <pico/time.h>


int main()
{
    stdio_init_all();

    int i=0 ,x = 0 , j = 0 ;
    char pass[6] = {'A','B','7','8','*','#'};
    char key  ; 
    init_gpio();
    lcd_initalise();
    lcd_string("The Password is :");
    transfer(LCD_CURSORSECONDLINE, LCD_COMMAND);
    transfer(LCD_CURSORINCREAMENT,LCD_COMMAND);
    while (true)
    {
        key = scan_keypad();
        if (key != '\0')
        {   
            lcd_char('*');
            printf("Key pressed: %c\n", key);

            if( key == pass[x++])
                j = j+1 ; 
            
        }
        sleep_ms(DELAY);
       
        if (j == 6 )
        {
            transfer(LCD_CLEARDISPLAY, LCD_COMMAND);
            lcd_string("Correct Password X");
        
            while (true)
            {
                key = scan_keypad();
                if (key != '\0')
                {
                    transfer(LCD_CLEARDISPLAY, LCD_COMMAND);
                    lcd_char(key);
                    printf("Key pressed: %c\n", key);
                }
                sleep_ms(DELAY/2);
            }
        }
        printf("The value of x = %d\n",x);
        if(x > 5)
        {
            x = j = 0;
            transfer(LCD_CLEARDISPLAY, LCD_COMMAND);
            lcd_string("Wrong Password X");
            sleep_ms(1000);
            transfer(LCD_CLEARDISPLAY, LCD_COMMAND);
            lcd_string("The Password is :");
            transfer(LCD_CURSORSECONDLINE, LCD_COMMAND);
            transfer(LCD_CURSORINCREAMENT,LCD_COMMAND);
        }
        
    }
}