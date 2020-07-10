#include "calculator.h"
#include "keypad_scanner.h"
#include "LCD_driver.h"
int key1;

void calculator(int number1){
			LCD_clear();
			int interrupt =1;
	    if (interrupt){
			if (key1 == 'A'){
			LCD_print_string("working");
			LCD_command(0xC0,0);
			LCD_print_number(number1);
		}
	}
}