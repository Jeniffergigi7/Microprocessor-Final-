/*
Some of the ADC initilization code is given. Fill in the rest of the code under each comment

You will need to refer to chapter 25 of the LPC1114 user manual for information about the ADC
Focus on the following sections:
	25.2 Basic Configuration (page 409)
	25.4 Pin description (page 409)
	25.5 Register description (page 410)
		25.5.1 A/D control register (page 411)
		25.5.3 A/D Interrupt enable register (page 413)
		25.5.4 A/D Data registers (page 413)
		
	Also, refer to section 7.4.28 on page 94 to set pin#4 as ADC mode
	Refer to page 34 to enable the clock for ADC
	Refer to page 45 to turn power onto the ADC


*/

#include "ADC.h"
#include "LPC11xx.h"


void adc_init(void)
{
		// set pin 4 (P0.11) as ADC 
		 LPC_IOCON->R_PIO0_11|= (1<<1);  //line 232 see page 94 
	
 	// turn off all resistors on pin 4
	// page 94 
	   LPC_IOCON->R_PIO0_11 &= ~(0x18);

	// Set the pin as analog mode (not digital) 
	// page 94 
		 LPC_IOCON->R_PIO0_11 &= ~(1<<7);
	
	 // Enable the clock and power for ADC 
	 // Refer to pages 34 and 45 
	   LPC_SYSCON -> SYSAHBCLKCTRL |= (1<<13); //enables the clock for ADC
	   LPC_SYSCON -> PDRUNCFG &= ~(1<<4);       //Powered the ADC_PD
	// the last two lines of code are supplied. 	
	
	// Enables AD[0], sets burst mode to 1 (hardware not software scan)
	// Also sets the clock speed to just under 4.5MHz
	  LPC_ADC->CR |= (0x10A01);    // check pages 410 -- 411
	
	// According to datasheet, bit 8 of the register below must be set to 0 if in burst mode
	  LPC_ADC->INTEN &= ~(1<<8);  // check page 413
}
