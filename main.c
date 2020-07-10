//					UART_send_char('\n'); // new-line
//					UART_send_char('\r'); // carriage_return
//					UART_send_char(c);   // can place char in a variable then pass the variable to the function

#include <system_LPC11xx.h>
#include <LPC11xx.h>
#include "LCD_driver.h"
#include "keypad_scanner.h"
#include "ADC.h"
#include "timer.h"
#include "UART.h"
#include "SPI.h"
#include <stdio.h> // for sprintf()
#include <string.h> // for memcpy()
#include <stdlib.h> // for atoi()

#define BUFFER_SIZE 20
// this is if you choose to use interrupt
int interrupt = 0; // global variable(s)
char key; 
char flag=1;

// global variables
char transmit_string_buffer[BUFFER_SIZE]; // no strings in c language, so use a char array
char UART_received_buffer[BUFFER_SIZE];
char received_string[BUFFER_SIZE];
char UART_received_flag = 0;
char UART_received_index = 0; 
char UART_received_byte = 0;
char degree_symbol = 0xDF;  // 0xDF is the degree symbol on Hitatchi HD44780 driver
int received_number=0;

//initialize the operands for the calculator
double operand1 = 0;
double operand2 = 0;
double result = 0;
int letter;
int operations;
int num = 0;
int getback=0;
int num1 =0;
char equal;

//initialize for the temperature measurement
int temp;
int sample=0;
int sample_rate = 0;
int sample_frequency =0;
int ADC_value;
int ADCtimer =0;
int counterADC =0;
int running_sum = 0;
int average=0;
int tempsum;
int repeat_temp;

//initialize for the SPI
int frequency;
int rx;
int resistx;
double value;
int first;
int counter;

/*******************************************************************
******** Timer Interrupt Handler ***********************************
*******************************************************************/
void TIMER32_0_IRQHandler(void)
{	
	LPC_TMR32B0->IR |= 1;  // do this first
	
	// now that the ADC is running, we must take a sample from it
	// We are using ADC[0] so we will read from the LPC_ADC->DR[0] register
	// The ADC value is stored in bits 15:6, so that is why a right shift by 6 occurs
	// Then & by 0x3ff to clear off any leading bits that are not of interest
	ADC_value = (LPC_ADC->DR[0] >> 6) & (0x3ff);  // check page 413
	
	temp = 3.33 /(1023.00/ADC_value) * 100;	//convert to celcius
	temp = temp *1.8 + 32;							//convert to Farenheit
	running_sum = running_sum +temp;

	counterADC++;
	if (counterADC == sample_rate)
	{
		tempsum = running_sum;
		ADCtimer=1;
	}
	
	// if you have modified the value in the match register (MR0), then you should reset the timer
	// If you have not, then the option to reset the timer is up to you 
	LPC_TMR32B0->TCR = 2;			   // reset timer
	LPC_TMR32B0->TCR = 1;				 // start timer
	
	if(key == 0x23){                                  //Press # to getback to prompt the user to 
				 operations=0;
				 repeat_temp= 2;
				 ADCtimer=0;
				 counterADC=0;
				 tempsum=0;
			 }	
}


/*******************************************************************
******** UART Receiving Interrupt Handler **************************
*******************************************************************/
// For the receiving handler, I have implemented it so that in your teminal window, 
// type all the characters you want in your message, then hit "Enter" on keyboard
// When the "Enter" char is received (0xD0), the UART received handler will set the 
//  UART_received_flag, and in the main() while(1) loop, the logic will begin
void UART_IRQHandler(void)
{
		UART_received_byte = LPC_UART -> RBR; // read in the byte
	
	if(UART_received_byte == 0x0D) // if "Enter" key was read in, set the flag since all data is received
	{
		UART_received_flag = 1; 
		
		// now use memcpy() function to convert the char array into a "string" (included in string.h)
		// this is usefull for comparing the string as shown in main()
		memcpy(received_string, UART_received_buffer, UART_received_index);
		received_string[UART_received_index] = '\0'; // place null character at end of string 
	}
	else // place the char into the string
	{
		
		if(UART_received_byte == '0' || UART_received_byte == '1' || UART_received_byte == '2' ||
			 UART_received_byte == '3' || UART_received_byte == '4' || UART_received_byte == '5' || 
		   UART_received_byte == '6' || UART_received_byte == '7' || UART_received_byte == '8' || 
		   UART_received_byte == '9' || UART_received_byte == 'A' || UART_received_byte == 'B' || 
		   UART_received_byte == 'C' || UART_received_byte == 'D' || UART_received_byte == '*' || 
       UART_received_byte == '#')
     	{ key = UART_received_byte;}

		UART_received_buffer[UART_received_index] = UART_received_byte;
		UART_received_index++; // increment the index by 1
	}	
}

// if you choose to use an interrupt based keypad scanner, use the function below as the interrupt handler
void PIOINT0_IRQHandler(void)  // interrupt handler for Port 0  (from LPC11xx.h)
{ 
	// this interrupt handler will be entered hundereds of times each time a button is pressed
	// so use "interrupt" as a flag so that the scan only happens once
	if(!interrupt)  
	{
		key = scanner();	
		interrupt = 1;
		// Reground rows 
	  groundRows();
	}
	
	// IMPORTANT: don't forget to reset interrupt in main
}

void TIMER32_1_IRQHandler(void)
{	
	//This timer interrupt handleris for the Timer B1 of 32bit counter
	
	LPC_TMR32B1->IR |= 1;  // do this first Interrupt the timer 
	
	//Toggle functon 
	if (flag){
		LPC_GPIO1->DATA &= ~0x200;  // by doing a bitwise NOT AND, the bit gets set to zero
		flag =0;                    //data for the PIO1_9 then make flag O 
	}
  else		// make high
	{
		LPC_GPIO1->DATA |= 0x200;    //It OR the data to find the ones and make flag 1
	  flag=1;
  }
}

void SPI_Timer(int freq){
	
	 // Enables clock for 32 bit timer (CT32B1)  Page 34
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10);  
		
		// timer mode not counter mode  (Page 367)
		LPC_TMR32B1->CTCR = 0; 
	
		// make the TC trigger an interrupt when MR0 matches with TC
		LPC_TMR32B1->MCR = 0x3;   // page 368
	
		// place value in match register MR0
		LPC_TMR32B1->MR0 = freq/2;   //The frequency enter divide  by 2.
	
		// enable the timer interrupt
		NVIC_EnableIRQ(TIMER_32_1_IRQn);
	
		LPC_TMR32B1->TCR = 2;			   // reset timer
		LPC_TMR32B1->TCR = 1;				 // start timer
	
}

//Calculator it will ask the user to enter the values for the second operand and which 
//if the user press # it will go to main to prin the result, if the user press *
//it will go to the main menu.
void calculator(void)
{
	int oper= 1;
	while(oper){
	if (interrupt||UART_received_flag){
		 key ='v';
		 operations=1;
	 	 while(operations)
		 {
  		  //for(int i=0 ; i< 9000; i++);					//DELAY	
			   if (key == 0x23){     					 //It is the # to display result
					equal=key;
					operations=0;
					oper=0;
					num1=0;
				  }
				if(key == 0x2A){
					interrupt=0;
					UART_received_flag = 0;   //Clear the flag 
			   	UART_received_index = 0;  //Reset the UART Received
					operations=0;
					oper=0;
					getback=0;
				}
			  if(key == '0' || key == '1' || key == '2' || key == '3' || 
					 key == '4' || key == '5' || key == '6' || key == '7' || 
				   key == '8' || key == '9'){
				  num1 = key - 0x30;
					operand2 =  10 * operand2 + num1;
				  LCD_print_char(key);
				  sprintf(transmit_string_buffer, "%d",num1);
				  UART_send_string(transmit_string_buffer);
				  key='v';					
				}	  
				interrupt =0;
				UART_received_flag = 0;   //Clear the flag 
				UART_received_index = 0;  //Reset the UART Received
			}
		}//end of the interrupt
	}//end of the infinity look
}//end the function

void resetvalues(void){
		operations=0;
		num=0;
		num1=0;
		getback=0;
		result=0;
		operand1=0;
		operand2=0;
		key='\0';
		sample=0;
	  repeat_temp = 0;
		value=0;
		rx=0;
		resistx=0;
}

void displaynumbers(void){
	for(int i=0 ; i< 1000; i++);					//DELAY	
	if(key == '0' || key == '1' || key == '2' || key == '3' || 
		 key == '4' || key == '5' || key == '6' || key == '7' ||
   	 key == '8' || key == '9'){
		num = key - 0x30;
		sample =  10 * sample+ num;
		LCD_print_char(key);
	  sprintf(transmit_string_buffer, "%d", num);
		UART_send_string(transmit_string_buffer);
		key='v';
	}//end of the if statement 
	}

void numbersSPI(void){
	for(int i=0 ; i< 1000; i++);				//DELAY	
	if(key == '0' || key == '1' || key == '2' || key == '3' || 
		 key == '4' || key == '5' || key == '6' || key == '7' || 
	   key == '8' || key == '9'||key == '#'){
		if (key == '#'){
		key = 0x2E;
		counter=-1;
		UART_send_string(".");
		LCD_print_char(key);
		key =0;
		}
		else
		num = key - 0x30;
		value =  10 * value+ num;

		if (key != 0){
		LCD_print_char(key);
	  sprintf(transmit_string_buffer, "%d", num);
		UART_send_string(transmit_string_buffer);
		key='v';}		
		counter++;
	}//end of the if statement 
	}

void displayADC(void){
	// now check if # samples have been taken
	// if so, average the samples and display on LCD
	if (counterADC >= sample_rate)
		{
			average = tempsum /sample;   // compute average
			LCD_clear();
			LCD_print_string("Temp ");				// print Temperature
			LCD_print_number(average);
			LCD_print_char(degree_symbol);  // print degree symbol
			LCD_print_string("F");
			LCD_command(0xC0,0);
			LCD_print_string("Sample #: ");
			LCD_print_number(sample_rate);
			sprintf(transmit_string_buffer, "%dF\n\r", average);
			UART_send_string(transmit_string_buffer);
			running_sum = 0;
			counterADC = 0;	
		}
		ADCtimer = 0;	
}

void displaymenu(){
//*******************************************************************//
	//                             MENU OPTIONS                          //
	//*******************************************************************//
	// to send a string, use sprintf() to place your string into a char buffer
	sprintf(transmit_string_buffer, "\n\nType the option you would like to do:\n\r");
	UART_send_string(transmit_string_buffer);
	sprintf(transmit_string_buffer, "A: Calculator\n\r");  
	UART_send_string(transmit_string_buffer);
	sprintf(transmit_string_buffer, "B: Temperature\n\r");  
	UART_send_string(transmit_string_buffer); 
	sprintf(transmit_string_buffer, "C: DDS and LPF\n\r");  
	UART_send_string(transmit_string_buffer);
	sprintf(transmit_string_buffer, "*: Return to Main\n\r");  
	UART_send_string(transmit_string_buffer);

	LCD_clear();
	LCD_print_string("A: Calculator");
	LCD_command(0xC0, 0);
	LCD_print_string("B: Temperature");
	for (int i=0;i<6400000;i++); //Delay
	LCD_command(0x02,0);
	LCD_print_string("C: DDS and LPF");
	LCD_command(0xC0, 0);
	LCD_print_string("*: Return Main");
}
	

int main()
{ 
	//*******************************************************************//
	// This part is the initialization for the keypad, LCD, and adc 
	//*******************************************************************//
	LCD_init();  
	LCD_clear();
	adc_init();   //go into ADC.c and complete the initialization code 
	keypad_init();
	UART_init();
	SPI_init();
	
	displaymenu();   //it will display the main menu
	
	 LPC_IOCON -> PIO1_9 |= 0x0;        // P1.9  set as GPIO
	 LPC_GPIO1 ->	DIR		 |=	0x200;	    //direction of the pin 
	 LPC_GPIO1 -> DATA   |= 0x200;      //set up as high
	
	
	//*******************************************************************//
	// This is a infinite loop where it will detect an interrupt from the
	// key or from the keyboard of the phone. It will check the temperature
	// by using different sample rates.
	//*******************************************************************//
	while(1)
	{
		//This flag will return the user to the main of temperature 
		if (repeat_temp==2)
		{
			interrupt=1;
			UART_received_flag = 0;   //Clear the flag 
			UART_received_index = 0;  //Reset the UART Received
		}
		
		//This will display the average temperature and the samples
		if (ADCtimer ==1){
				displayADC();
		}	

//**************************************************************************************//
// This is a loop that runs when it detects the key pressed or a type is send.
//**************************************************************************************//
		for(int i=0;i<9000;i++);           //Delay
		while((interrupt) || (UART_received_flag))
			{
//-----------------------------------INTERRUPT------------------------------------------//				
		if (interrupt || UART_received_flag){
//***************************************************************************************//
//************************************CALCULATOR*****************************************//		
// 16 bit Calculator enter the first operand A = '+', B = '-', C = '*', D = '/' 
// Push # to calculate	
			if(key == 0x41){
				UART_received_flag=0;
				getback=1;
				while(getback){
					key = '\0';
					LCD_clear();
					LCD_print_string("   CALCULATOR");
					LCD_command(0xC0, 0);
					sprintf(transmit_string_buffer, "CALCULATOR\n\r");
	        UART_send_string(transmit_string_buffer);
					LCD_print_string("A:+,B:-,C:*,D:/");
					sprintf(transmit_string_buffer, "A:+,B:-,C:*,D:/ \n\r");
	        UART_send_string(transmit_string_buffer);
				
					for(int i=0 ; i< 9200000; i++);					//DELAY		
						
					LCD_clear();
				  operations = 1;
					while(operations){
						displaynumbers();
						operand1=sample;
						if(key == '*'){
							resetvalues();
							displaymenu();
						}
//------------------------Addition------------------------
						if(key == 0x41){                     
							LCD_print_char(0x2B);            //display +
							sprintf(transmit_string_buffer, "+");
	            UART_send_string(transmit_string_buffer);
							calculator();
							if (key == '*'){
								resetvalues();
								displaymenu();
							}
							if (equal == '#'){
							result = operand1+operand2;
							LCD_command(0xC0,0);
							LCD_print_char(0x3D);
							sprintf(transmit_string_buffer, "=");
	            UART_send_string(transmit_string_buffer);
							LCD_print_number(result);
							sprintf(transmit_string_buffer, "%0.f",result);
						  UART_send_string(transmit_string_buffer);
							resetvalues();
							operations=1;
							while(operations){
							interrupt=0;
							UART_received_flag = 0;   //Clear the flag 
				      UART_received_index = 0;  //Reset the UART Received
							if(key == '*'){
								resetvalues();
								displaymenu();
							}
						  else if (key){
									operations=0;
									getback=1;
									key='v';
								}//end the if statement
							 }//end the while loop
							} //end of the result if statement 							
						 }//end Addition
//---------------------Substraction------------------------
						else if(key == 0x42){     
              LCD_print_char(0x2D);						  //display -	
							sprintf(transmit_string_buffer, "-");
	            UART_send_string(transmit_string_buffer);
							calculator();
							if(key == '*'){
								resetvalues();
								displaymenu();
							}
							else if(equal == '#'){
							result = operand1-operand2;
							LCD_command(0xC0,0);
							LCD_print_char(0x3D);
							sprintf(transmit_string_buffer, "=");
	            UART_send_string(transmit_string_buffer);
							LCD_print_number(result);
							sprintf(transmit_string_buffer, "%0.f",result);
						  UART_send_string(transmit_string_buffer);
	          	resetvalues();
							operations=1;
							while(operations){
								interrupt=0;
						    UART_received_flag = 0;   //Clear the flag 
				        UART_received_index = 0;  //Reset the UART Received
								if(key == '*'){
									resetvalues();
								  displaymenu();
								}
								else if (key){
									operations=0;
									getback=1;
									key = 'v';
								}//end the if statement 
							}//end the while loop
						 }//end if the result statement
						}//end Substraction
//---------------------Multiplication----------------------
						else if(key == 0x43){     
							LCD_print_char(0x2A);           //display *
							sprintf(transmit_string_buffer, "*");
	            UART_send_string(transmit_string_buffer);
							calculator();
							if(key == '*'){
									resetvalues();
								  displaymenu();
							}
							else if(equal == '#'){
							result = operand1*operand2;
							LCD_command(0xC0,0);
							LCD_print_char(0x3D);
							sprintf(transmit_string_buffer, "=");
	            UART_send_string(transmit_string_buffer);
							LCD_print_number(result);
							sprintf(transmit_string_buffer, "%0.f",result);
						  UART_send_string(transmit_string_buffer);
							resetvalues();
							operations=1;
							while(operations){
								interrupt=0;
			          UART_received_flag = 0;   //Clear the flag 
			        	UART_received_index = 0;  //Reset the UART Received
								if(key == '*'){
									resetvalues();
								  displaymenu();
								}
								else if (key){
									operations=0;
									getback=1;
									key = 'v';
							 }//end the if statement 
							}//end the while loop
						 }//end if the result statement
						}//end Multiplication
//------------------------Division-------------------------
						else if(key == 0x44){     
              LCD_print_char(0x2F);						  //display /
							sprintf(transmit_string_buffer, "/");
	            UART_send_string(transmit_string_buffer);
							calculator();
							if (key == '*'){
								resetvalues();
								displaymenu();
							}
							else if(equal == '#'){
							result = operand1/operand2;
							LCD_command(0xC0,0);
							LCD_print_char(0x3D);
							sprintf(transmit_string_buffer, "=");
	            UART_send_string(transmit_string_buffer);
							LCD_print_float(result);
							sprintf(transmit_string_buffer, "%2.f",result);
						  UART_send_string(transmit_string_buffer);
							resetvalues();
							operations=1;
							while(operations){
								interrupt=0;
								UART_received_flag = 0;   //Clear the flag 
			        	UART_received_index = 0;  //Reset the UART Received
								if (key == '*'){
								resetvalues();
								displaymenu();
								}
								else if (key){
									operations=0;
									getback=1;
									key = 'v';
								}//end the if statement 
							}//end the while loop
						 }//end if the result statement
						}//end Division
						interrupt =0;
            UART_received_flag = 0;   //Clear the flag 
						UART_received_index = 0;  //Reset the UART Received
						}//end of the while loop
					}//end while loop get back
				} //end for the calculator
//***************************************************************************************//
//***************************************************************************************//			
//************************************TEMPERATURE*****************************************//					
			if (key == 0x42 || repeat_temp == 2){
				resetvalues();
				//Temperature measurements
				LCD_clear();
				LCD_print_string("Num Frequency:");
				LCD_command(0xC0, 0);
				sprintf(transmit_string_buffer, "\n\rEnter Num Frequency\n\r");
				UART_send_string(transmit_string_buffer);
				operations=1;
				while(operations){
					displaynumbers(); 						//call function to display numbers in LCD
					if(key == '*'){
						resetvalues();
						displaymenu();
					}
					if(key==0x43){  							//Press D to save the value of the frequency
						key = '\0';
						sample_frequency=sample;
						sample_frequency= 48000000/sample_frequency;
						sample=0;
						LCD_clear();
						LCD_print_string("Num Samples:");         //Display to the user to enter sample #
						LCD_command(0xC0, 0);
						sprintf(transmit_string_buffer, "\n\rEnter Num Samples\n\r");
	          UART_send_string(transmit_string_buffer);
						displaynumbers();  						        	//call function to display in LCD
						if(key == '*'){
						resetvalues();
						displaymenu();
						}
					}
					if(key == 0x23){                                  //Press # to getback to prompt the user to 
						operations=0;
						repeat_temp= 2;
					  counterADC=0;
					  tempsum=0;
					}	
					if(key == 0x44){                                    //Press D to start sampling
						LCD_clear();
						counterADC=0;
						running_sum=0;
						sample_rate=sample;                           //save the sample #enter to sample_rate
						timer_init(sample_frequency);                 //start sampling the values. It will go to time
						operations=0;
						}
					if (key == '*'){
						resetvalues();
						displaymenu();
					}
					interrupt=0;
					UART_received_flag = 0;   //Clear the flag 
				  UART_received_index = 0;  //Reset the UART Received
				}//end of the while operations 
			}//end of the temperture
//***************************************************************************************//
//***************************************************************************************//	
//***************************GENERATOR SQUARE WAVE***************************************//			
			if (key == 0x43){
							//Function generator and potentiometer control
							//Enter a frequency between 100Hz to 10KHz
							//Enter the gain value
							resetvalues();
							LCD_clear();
							LCD_print_string("Enter Frequency:");
							LCD_command(0xC0, 0);
							sprintf(transmit_string_buffer, "\n\rEnter Frequency\n\r");
							UART_send_string(transmit_string_buffer);	
							first=1;
							operations=1;
							interrupt=0;
							UART_received_flag = 0;   //Clear the flag 
			      	UART_received_index = 0;  //Reset the UART Received
							while(operations){
								if (first==1){
								displaynumbers(); 						//call function to display numbers in LCD
								}
								else if (first ==2){
									numbersSPI();
								}
								if(key == '*'){
									resetvalues();
									displaymenu();
								}
								if (key == 'C'){
									key='\0';
									frequency=sample;
									frequency=48000000/frequency;
									sample=0;
									LCD_clear();
							    LCD_print_string("Enter Gain");
									LCD_command(0xC0, 0);
							    sprintf(transmit_string_buffer, "\n\rEnter Gain\n\r");
									UART_send_string(transmit_string_buffer);	
									first=2;
									if(key == '*'){
										resetvalues();
										displaymenu();
									}
								}
								if (key == 'D'){
									if (counter==1){
											value=value/10;
									}
									//else if (counter==2){
									//		value= value/100;
								//	}
									else if(counter ==3){
									value= value/1000;
									}
									SPI_Timer(frequency);
									resistx = value*1000;       //find the resistance rx
									rx = resistx;
									rx = (rx/37);           //resistance per spi 
									if (rx > 255){        //check if it is greater than 255
									rx=255;
									}
									set_resistance(rx);
									LCD_clear();
									LCD_print_string("Resistance x");
									LCD_command(0xC0,0);
									LCD_print_char(0x3D);
									LCD_print_number(resistx);
									sprintf(transmit_string_buffer, "\n\rResistance x\n\r");
									UART_send_string(transmit_string_buffer);		
									sprintf(transmit_string_buffer, "=");
									UART_send_string(transmit_string_buffer);
									sprintf(transmit_string_buffer, "%d",resistx);
									UART_send_string(transmit_string_buffer);
									key='\0';
									if(key == '*'){
										resetvalues();
										displaymenu();
									}
									
								}
								interrupt=0;
								UART_received_flag = 0;   //Clear the flag 
			        	UART_received_index = 0;  //Reset the UART Received
							}
			}
//***************************************************************************************//
//***************************************************************************************//	
//**************************************RESET********************************************//		
			if (key == 0x2A){
				 		 //Reset and return to main at any time
							displaymenu();
						  resetvalues();
					   }
					 }	// end interrupt 
					interrupt=0;
					UART_received_flag = 0;   //Clear the flag 
				  UART_received_index = 0;  //Reset the UART Received
				 } //end of the while loop detects interrupt or UART signal
			 } //end of the infite loop
	return 0;
			 UART_received_flag = 0;   //Clear the flag 
			 UART_received_index = 0;  //Reset the UART Received
}