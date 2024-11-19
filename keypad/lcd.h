#include "keypad.h"

//Macro for LCD
#define ADDR 0x27
#define SIZE 1
#define DELAY 200

//Defination pins for I2c
#define PICO_I2C_SDA_PIN 16
#define PICO_I2C_SCL_PIN 17

// Modes for excute_command
#define LCD_CHARACTER  1
#define LCD_COMMAND    0

// commands for LCD
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME  0x02
#define LCD_ENTRYMODESET  0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT  0x10
#define LCD_FUNCTIONSET  0x20
#define LCD_4BITDATA  0x28
#define LCD_SETCGRAMADDR  0x40
#define LCD_SETDDRAMADDR  0x80

// flags for display and cursor control
#define LCD_BLINKON 0x01
#define LCD_CURSOROFF 0x0c
#define LCD_DISPLAYON 0x04
#define LCD_CURSORINCREAMENT 0x06
#define LCD_CURSORFIRSTLINE 0x80 
#define LCD_CURSORSECONDLINE 0x0c1 

// flag for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_BACKLIGHTOFF 0x00
//enable to write on LCD
#define LCD_ENABLE_BIT 0x04

void high_low_enable(uint8_t val);
void transfer(uint8_t val , int mode);
static inline void lcd_char(char val);
void lcd_string(const char *s);
void lcd_initalise();
