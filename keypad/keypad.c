#include "keypad.h"

// Define the GPIO pins for rows and columns
uint row_pins[ROWS] = {2,3,4,5};
uint col_pins[COLS] = {6,10,8,11};

// Define the keymap
char keys[ROWS][COLS] =
{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};


// Function to initialize the GPIO pins
void init_gpio()
{
  // This example will use I2C0 on the SDA and SCL pins (16, 17 on a Pico)
    i2c_init(i2c0, 400 * 1000); //400 KHz
    gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_I2C_SDA_PIN); 
    gpio_pull_up(PICO_I2C_SCL_PIN);
    for (int i = 0; i < ROWS; i++)
    {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], GPIO_OUT);
        gpio_put(row_pins[i], LOW);
    }
    for (int i = 0; i < COLS; i++)
    {
        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], GPIO_IN);
        gpio_pull_down(col_pins[i]);
        gpio_put(col_pins[i], LOW);
    }
}

// Function to scan the keypad
char scan_keypad()
{
    for (int row = 0; row < ROWS; row++)
    {
        gpio_put(row_pins[row], HIGH);
        for (int col = 0; col < COLS; col++)
        {
            if (gpio_get(col_pins[col]) == HIGH)
            {
                gpio_put(row_pins[row], LOW);
                return keys[row][col];
            }
        }
        gpio_put(row_pins[row], LOW);
    }
    return '\0';
}
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
static inline void lcd_char(char val) {
    transfer(val, LCD_CHARACTER);
}
void display_string(const char *s) {
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

int main()
{
    stdio_init_all();

    char key ; 
    init_gpio();
    lcd_initalise();
 
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

