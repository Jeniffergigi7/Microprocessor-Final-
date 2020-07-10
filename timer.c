#include "timer.h"

void timer_init(int rate)
{
		// Enables clock for 32 bit timer (CT32B0)  Page 34
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);  
		
		// timer mode not counter mode  (Page 367)
		LPC_TMR32B0->CTCR = 0; 
	
		// make the TC trigger an interrupt when MR0 matches with TC
		LPC_TMR32B0->MCR = 0x3;   // page 368
	
		// place value in match register MR0
		LPC_TMR32B0->MR0 = rate;  //  value that the user enters
	
		// enable the timer interrupt
		NVIC_EnableIRQ(TIMER_32_0_IRQn);
	
		LPC_TMR32B0->TCR = 2;			   // reset timer
		LPC_TMR32B0->TCR = 1;				 // start timer
	
}
