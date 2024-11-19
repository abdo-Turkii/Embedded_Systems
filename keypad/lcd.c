#include "keypad.h"

void high_low_enable(uint8_t val)
{
    sleep_us(500);
    uint8_t enable = val | LCD_ENABLE_BIT;
    uint8_t disenable = val & ~LCD_ENABLE_BIT;

    i2c_write_blocking(i2c0, ADDR, &enable, SIZE, false);
    sleep_us(500);
    i2c_write_blocking(i2c0, ADDR, &disenable, SIZE, false);
    sleep_us(500);
}
void transfer(uint8_t val , int mode)
{
    uint8_t most = mode | (val & 0xF0) | LCD_BACKLIGHT ;
    uint8_t last = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT ;

    i2c_write_blocking(i2c0, ADDR, &most, SIZE, false);
    high_low_enable(most);
    i2c_write_blocking(i2c0, ADDR, &last, SIZE, false);
    high_low_enable(last);
}
void lcd_char(char val)
{
    transfer(val, LCD_CHARACTER);
}
void lcd_string(const char *s)
{
    while (*s)
    {
       lcd_char(*s++);
    }
}
void lcd_initalise()
{
    sleep_ms(40);
    transfer(LCD_RETURNHOME,LCD_COMMAND);
    transfer(LCD_4BITDATA,LCD_COMMAND);
    transfer(LCD_CURSORINCREAMENT,LCD_COMMAND);
    transfer(LCD_CURSOROFF,LCD_COMMAND);              //display on cursor off
    transfer(LCD_CLEARDISPLAY,LCD_COMMAND);
}

