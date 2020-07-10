/****************************************************************
  This code was provided to me from Professor Yi Zheng. 
	Modifications were made in terms of the ports and pins used
****************************************************************/
/*
	PORTS and PINS
	D4: P0.4
	D5: P0.3
	D6: P0.2
	D7: P0.1
	RS: P0.7
	E:  P1.8
	R/W: GROUNDED
*/

#include "LCD_driver.h"

#define data_line (0x11A) // 0001_0001_1010  (D4-D7 are on pins 0.1, 0.2, 0.8, and 0.4)
#define RS (0x80)     // 1000_0000 (RS is on pin 0.7) 
#define E (0x100)     // 1_0000_0000 (Enable is on pin 1.8) 



/********************************************************************************************************/
void LCD_command(int code, int printLetter)
{
	/*
	   First, send in the top 4 bits from "code"
	   Next, send in the bottom 4 bits from "code"
	
		 This code follows the timing sequence shown in the LCD's user manual
	*/
	//For the sharing pins, it will disable the pins for the LCD and keypad 
	NVIC-> ICER[0] = (1<< 31);  // disable interrupt four pins
	LPC_GPIO0 ->DIR |=(0x11A); 
	
	// if code = 0x45, then topHalf = 0x04 and bottomHalf = 0x05
	int topHalf = (code >> 4)& 0xF; 
	int bottomHalf = code & 0xF;    
	int delay = 9000; //The amount of delay between each sequence
	int i; // delay loop index
	
	
	LPC_GPIO0 ->DATA &=~(data_line); // Clear data lines
	LPC_GPIO0 ->DATA &= ~(RS);			 // clear RS
	LPC_GPIO1 ->DATA &= ~(E);        // clear E
	
	
	if (printLetter) //If printLetter = 1, put LCD in data mode (not command mode)
	{
		LPC_GPIO0 ->DATA |=(RS); //Set the RS pin High
	}
	
	for (i=0;i<delay;i++); //Time delay
	
	// We are now ready to send data to the LCD. 
	// According to the LCD's user manual, the MSB is to be put into D7, and LSB into D4 
	
	LPC_GPIO0 ->DATA |= (topHalf & (1<<3)) >> 2;   // puts MSB onto D7 (P0.1)
	// shifted right by 3 when doing the & so that the MSB of topHalf is extracted
	// then it is right shifted by two so that it goes into bit 1 of GPIO0->DATA (P0.1)
	
	// send D6-D4
	LPC_GPIO0 ->DATA |= ((topHalf & (1<<2)) << 6);        // puts next bit onto D6 (P0.8)
	LPC_GPIO0 ->DATA |= (topHalf & (1<<1)) << 2;    // puts next bit onto D5 (P0.3)
	LPC_GPIO0 ->DATA |= (topHalf & 1) << 4;				 // puts LSB onto D4 (P0.4)

	
	
	LPC_GPIO1 ->DATA |=(E); //  Put E high

	for (i=0;i<delay;i++); //Time delay
	
	LPC_GPIO1 ->DATA &=~(E); // Put E low
	
	
	// We now need to send the bottom half of the data. First, clear the data line
	LPC_GPIO0 ->DATA &= ~(data_line); 					  	// clears Data line	 
	
	LPC_GPIO0 ->DATA |= (bottomHalf & (1<<3)) >> 2;   // puts MSB onto D7 (P0.1)
	LPC_GPIO0 ->DATA |= ((bottomHalf & (1<<2)) << 6);        // puts next bit onto D6 (P0.8)
	LPC_GPIO0 ->DATA |= (bottomHalf & (1<<1)) << 2;    // puts next bit onto D5 (P0.3)
	LPC_GPIO0 ->DATA |= (bottomHalf & 1) << 4;				 // puts LSB onto D4 (P0.4)
	
	
	LPC_GPIO1 ->DATA |=(E); //  Put E high

	for (i=0;i<delay;i++); //Time delay
	
	LPC_GPIO1 ->DATA &=~(E); // Put E low
	
	
	LPC_GPIO0 ->DATA &= ~(data_line); //Reset Data pins
	
	//It needs to enable the four pins 
	LPC_GPIO0 ->DIR &=~(0x11A);
	NVIC-> ISER[0] = (1<< 31);	//enable interrupt
}

/********************************************************************************************************/

void LCD_init (void)
{

	/*
	
	A few commonly used commands for the LCD. All of the commands can be found in the datasheet.
	CLEAR       EQU		0X01
	HOME        EQU		0X02
	LCD_ON      EQU		0X0C
	BLINK       EQU		0X0F
	CURSOR_ON   EQU		0X0E
	LEFT        EQU		0X10
	RIGHT       EQU		0X14
	NEXT_LINE   EQU		0xC0
	FOURBIT     EQU		0x28   
	LCD_OFF     EQU		0x0A
	
	
	PORTS and PINS
	D4: P0.4
	D5: P0.3
	D6: P0.2
	D7: P0.1
	RS: P0.7
	E:  P1.8
	R/W: GROUNDED
	
	*/

	// ------------ INITIALIZE DATA BITS ---------------------
	// All six pins used by LCD are defaulted as GPIO pins
	LPC_GPIO0->DIR |= data_line;  // data_line defined at top of this file 
	
	
	
	// ------------- INITALIZE CONTROL BITS --------------------
	
	 // Sets RS and E as OUTPUTS
	LPC_GPIO0->DIR |= RS;  
	LPC_GPIO1->DIR |= E;

	// ---------------------------------------------------------
	
	
	LPC_GPIO0 ->DATA &=~(data_line); // Clear data lines
	LPC_GPIO0 ->DATA &= ~(RS);			 // clear RS
	LPC_GPIO1 ->DATA &= ~(E);        // clear E
	
	int i;                      //Index variable for delay
	for (i=0;i<100000;i++);     //Let the LCD warm up
	
	// Initialization sequence for the LCD. This clears, and sets up the LCD
	LCD_command(0x33,0); // Startup command
	LCD_command(0x32,0); // Startup command
	LCD_command(0x28,0); // Fourbit
	LCD_command(0x0F,0); // bink
	LCD_command(0x01,0); // clear
	LCD_command(0x02,0); // home
	LCD_command(0x0C,0); // no blinking
}

/********************************************************************************************************/
// The code below this is for functions that you can call in your program. refer to main() for a sample
//    of how to  use each one of these commands

/********************************************************************************************************/


void LCD_cursor (int x, int y) 
{
	/*
	The LCD_cursor function will reset the cursor location and then move the cursor to the desired location.
	Note: The first location on the LCD is represented as (0,0)
	
	Inputs
		x - LCD column
		y - LCD row
	*/
	
	LCD_command (0x02,0); //Set to home position
	if (y == 1)
	{
		LCD_command (0xC0,0); //Next Line
	}
	for (int i=0; i < x; i++) // muve cursor right as many times as x is numbered
	{
		LCD_command (0x14,0); //Move cursor right one
	}
}

/********************************************************************************************************/

void LCD_print_string(char* str)
{
	
	while(*str != '\0') {
		LCD_command(*str,1);
		str++;
	}
}


void LCD_clear(void)
{
	LCD_command (0x01,0); //Set to home position
}

void LCD_print_char(char op)
{
	LCD_command(op, 1);	
}

void LCD_print_number(int number)
{
	char buffer[11];
	sprintf(buffer, "%d", number); // turns an integer into a string 
	LCD_print_string(buffer);
}

void LCD_print_float(float number)
{
	char buffer[11];
	sprintf(buffer, "%.2f", number);  // turns a float into a string
	LCD_print_string(buffer);
}
