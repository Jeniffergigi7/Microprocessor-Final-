#include "SPI.h"

void SPI_init()
{
	// First, configure the 3 SPI pins
	// Page 172 -- Pin Layout
	
	// SCK pin needs to be mapped to Pin 6 (see page 104)
	LPC_IOCON ->SCK_LOC |= 0x2; // maps it to pin 6
	LPC_IOCON -> PIO0_6 |= 0x2; // selects SCK0 mode (page 88)
	
	// MOSI pin
	LPC_IOCON -> PIO0_9 |= 0x1; // sets pin to MOSI mode (page 91)
	
	// SSEL pin 
	LPC_IOCON -> PIO0_2 |= 0x1; // sets pin to SSEL0 mode (page 81)
	
	
	// Now, time to turn on the power as it states on page 225
	LPC_SYSCON -> SYSAHBCLKCTRL |= (1<<11); // (page 34)
	
	// Now enable the SPI0 peripheral clock as it states on page 225
	LPC_SYSCON -> SSP0CLKDIV |= 1; // page 35, simply setting it to 1 
	
	// Set bits 0 and 2 in the PRESETCTRL register to 1 per page 225
	LPC_SYSCON -> PRESETCTRL |= 0x5; // page 28, sets bits 0 and 2 to 1
	
	
	// Now, its time to configure the SPI0 specific registers
	// You need to refer to the MPC41010 datasheet for specific information
	LPC_SSP0 -> CR0 |= 0xF; // we need 16 bit transfer
	
	LPC_SSP0 -> CPSR |= 0x2; // sets clock to be maximum value
	
	LPC_SSP0 -> CR1 |= 0x2; // enables the SPI controller
}

void set_resistance(int value)
{
	// the SPI on this chip features a 16 bit transmission FIFO
	// According to MCP41010 datasheet, the first byte written is a command byte
	// The command byte is 0x11 to Write to Potentiometer 0 (this digi pot only has 1, some have 2)
	
	// before we send the command and the data to the digi-pot, ensure the SPI is ready by
	// checking that the BSY bit of the SR register is 0
	while((LPC_SSP0 -> SR & (1<<4)) == 1) {}; // waits for the bit to go to 0
		
	// send the command byte (offset by 1 byte) and the value
	LPC_SSP0 -> DR = (0x11 << 8) + value; 
		
	// before we exit, ensure that the SPI transfer is complete
	while((LPC_SSP0 -> SR & (1<<4)) == 1) {}; // waits for the bit to go to 0	
}

void MAX_resistance(void)
{
	while((LPC_SSP0 -> SR & (1<<4)) == 1) {}; // waits for the bit to go to 0
		
	LPC_SSP0 -> DR = (0x11 << 8) + 0xFF; // FF is max resistance of ~10k Ohms 
		
	while((LPC_SSP0 -> SR & (1<<4)) == 1) {}; // waits for the bit to go to 0		
}

void MIN_resistance(void)
{
	while((LPC_SSP0 -> SR & (1<<4)) == 1) {}; // waits for the bit to go to 0
		
	LPC_SSP0 -> DR = (0x11 << 8) + 0x00; // 0x00 is minumum resitance of ~50 Ohms 
		
	while((LPC_SSP0 -> SR & (1<<4)) == 1) {}; // waits for the bit to go to 0		
}

