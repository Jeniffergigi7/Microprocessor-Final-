#include "keypad_scanner.h"
#include "LPC11xx.h"



/*
Columns: 
				 P0.8
				 P0.9
				 P0.5
				 P0.6
				 
Rows:
				 P1.0
				 P1.1
				 P1.2
				 P1.4
*/




#define rows 0x17
#define columns 0x11A   //shared the pins of the keypad 

// function that will ground all the rows
void groundRows()
{
	LPC_GPIO1->DATA &= ~(rows);
}


/****************************************************************************************
  This function will set the pins as GPIO, INPUTS/OUTPUTS, and will ground the rows
****************************************************************************************/
void keypad_init(void)
{
	//This is detect the columns when it does not shar epins with LCD
	// First, declare the 8 pins as GPIO (LPC_IOCON-> .... )
	//Set the pins as inputs which are the columns
	// bits 2:0 must be 0x0 to be in GPIO
	//(NOTE: pins default to input)
  //	LPC_IOCON->PIO0_8|= 0x0;  //line 227
  //	LPC_IOCON->PIO0_9|= 0x0;  //line 228
  //	LPC_IOCON->PIO0_5|= 0x0;  //line 214
  //	LPC_IOCON->PIO0_6|= 0x0;  //line 221
	
	// bits 2:0 must be 0x0 || Ox1 to be in GPIO
	//Set the pins as outputs which are the rows
	LPC_IOCON->R_PIO1_0|= 0x1;   //line 233
	LPC_IOCON->R_PIO1_1 |= 0x1;  //line 234
	LPC_IOCON->R_PIO1_2 |= 0x1;  //line 236
	LPC_IOCON->PIO1_4 |= 0x0;    //line 241

	// Next, set the columns as inputs (NOTE: pins default to input)
  LPC_GPIO0->DIR|=(0x00);
	//Set the rows as outputs
	LPC_GPIO1->DIR |= 0x17;  //setting up as output Pin1_0 to Pin1_4
	
	// Finally, ground the four rows (LPC_GPIO1-> ...)
	groundRows();	
	// If you choose to use an interrupt, set it up here
	// modify the LPC_GPIO0->IS register to set as a level sensitve interrupt page 193
	LPC_GPIO0 ->IS |= (0x11A); 			// PIN0_5-PIN0_6, PIN0_8-PIN0_9 as level sensitive interrupt
	// modify the LPC_GPIO0->IEV register to set as active low
	LPC_GPIO0 ->IE |= (0x11A); 			// PIN0_5-PIN0_6, PIN0_8-PIN0_9 as interupt triggering 
	//low level since IS is level sensitive 
  LPC_GPIO0 ->IEV |=0x0;
	//set the columns to trigger the interrupt: NVIC_EnableIRQ(EINT0_IRQn); 
	NVIC ->ISER[0] |=(1<<31);   //Enable interrupt for port 0
  
}

char scanner(void)
{
	char letterArray[4][4] = {
		{'1','2','3','A'},
		{'4','5','6','B'},
		{'7','8','9','C'},
		{'*','0','#','D'}};
	
	int scanCodes[4] = {0x16,0x15,0x13,0x7};  // to check the rows R1,R2,R3,R4
	int row;
	int i;
	
	for (i=0;i<1000;i++); //Delay
	
	for (row = 0;row < 4; row++)
	{
		//int checkrow = LPC_GPIO1 ->DATA;    //get the data from the Port1 
		//checkrow |= ~(0x360);
		LPC_GPIO1 ->DATA = (scanCodes[row]);
		switch((LPC_GPIO0 ->DATA)&(0x11A))                
		{
			case 0x10A :  //100001010 Column 1
				LPC_GPIO0 ->DATA &= (0x17);
				//for (i=0;i<1000;i++); //Delay
				return letterArray[row][0];
			case 0x112 : //100010010 Column 2
				LPC_GPIO0 ->DATA &= (0x17);
				for (i=0;i<500;i++); //Delay
				return letterArray[row][1];
			case 0x1A : //11010 Column 3
				LPC_GPIO0 ->DATA &= (0x17);
				//for (i=0;i<1000;i++); //Delay
				return letterArray[row][2];
			case 0x118 : //100011000 Column 4
				LPC_GPIO0 ->DATA &= (0x17);
				//for (i=0;i<1000;i++); //Delay
				return letterArray[row][3];
		}
	}
	groundRows();
	return 'x';
}