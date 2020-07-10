#include <LPC11xx.h>			
#include "system_LPC11xx.h"
#include <stdio.h>


// Function Prototypes
void LCD_command(int code, int printLetter);
void LCD_init (void);
void LCD_cursor (int x, int y);
void LCD_print_string(char* str);
void LCD_clear(void);
void LCD_print_char(char op);
void LCD_print_number(int number);
void LCD_print_float(float number);

