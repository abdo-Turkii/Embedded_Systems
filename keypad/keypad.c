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
