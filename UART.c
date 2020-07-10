#include "UART.h"

// Refer to chapter chapter 13 of LPC1114 user manual for UART info

void UART_init(void)
{
	// Step 1: enable pin 15 to be RX (page 102)
	LPC_IOCON -> PIO1_6 |= 0x1; // mode is now RX
	
	// Step 2: enable pin 16 to be TX (page 
  LPC_IOCON -> PIO1_7 |= 0x1; // mode is now TX
	
		// Turn on power for UART
	// refer to page 34 
	LPC_SYSCON -> SYSAHBCLKCTRL |= (1<<12); 
	
	// Enable clock for UART
	// for now, just divide by 1 
	// page 35
	LPC_SYSCON -> UARTCLKDIV = 1;
	
	// For UART, it is convienent to use an interrupt when RECEIVING data, but not transmitting data
	// So enable UART received interrupt
	LPC_UART -> IER |= 1; 
	NVIC_EnableIRQ(UART_IRQn);
	
	// We will be receiving/transmitting 8-bit characters, so we need to 
	//   specify that in the UART->CR register (page 206)
	// Also, we don't use the parity bit
	LPC_UART -> LCR |= 0x3; 
	
	
	// now we need to configure the BAUD rate clock
	// formula is on page 216
	//    PCLK = 48MHz
	//    use BAUD rate of 96000
	//    Refer to page 219 for more calculation info + a table 
	//    48,000,000 / (16 * 115200) = 26.042
	//    fr needs to be between 1 to 1.9
	//    let DL = 24
	//    then fr = 1.04
	//    so use DLM = 0, DLL = 24, DivAddVal = 1, MulVal = 15
	LPC_UART -> FDR |= 0x1; // sets DivAddVal to 1
	LPC_UART -> FDR |= 0xF0; // sets MulVal to 15
	LPC_UART -> LCR |= (1<<7); // must enable the Divisor Latch before modifying DLL and DLM
	LPC_UART -> DLM |=  1;
	LPC_UART -> DLL |= 39;
	LPC_UART -> LCR &= ~(1<<7); // disable the Divisor Latch
	
	// at this point, the baud rate should be 115200, but actual value is 117187
	// error rate is ~1.7%, which should be good enough
	
	
	// configure the FIFOs (206)
	LPC_UART -> FCR |= 1; // enables the Rx and Tx FIFOs
	
	
	// Now enable UART transmission (page 219)
	LPC_UART -> TER |= (1<<7);
	
}


void UART_send_char(char c)
{
	LPC_UART -> THR = c;
}

// for sending a string, just send each index individually to the send_char function
void UART_send_string(char string[])
{
	int i = 0;
	
	while(string[i] != '\0')
	{
		if(LPC_UART -> LSR & (1<<5)) // wait until the THR is empty before trying to write next byte
		{
			UART_send_char(string[i]);
			i++;
		}
	}
	
}



