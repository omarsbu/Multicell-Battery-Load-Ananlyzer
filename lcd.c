/*
 * lcd.c
 *
 * Created: 12/18/2024 4:01:41 PM
 *  Author: tyler
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

//***************************************************************************
//
// Function Name : "lcd_spi_transmit"
// Target MCU : AVR128DB48
// DESCRIPTION
// Transmits an ASCII character to the LCD display using
// the SPI interface
//
// Inputs : char cmd: the character to be transmitted to the LCD
//
// Outputs : none
//
//
//**************************************************************************
void lcd_spi_transmit (char cmd)
{
	VPORTA_OUT &= ~PIN7_bm; //set PA7 to 0 to enable LCD Slave
	SPI0_DATA = cmd;		//send command
	while(!(SPI0_INTFLAGS & SPI_IF_bm)) {}    // wait until Tx complete
	VPORTA_OUT |= PIN7_bm; //set PA7 to 1 to disable LCD Slave
	_delay_us(100); //delay for command to be processed
}

//***************************************************************************
//
// Function Name : "init_spi_lcd"
// Target MCU : AVR128DB48
// DESCRIPTION
// Initializes and enables the SPI0 module, disables slave select,
// enables master mode, sets the mode to SPI mode 3, and sets
// the SPI0 GPIO pins to outputs
//
// Inputs : none
//
// Outputs : none
//
//
//**************************************************************************
void init_spi_lcd (void)
{
	VPORTA_DIR |= (PIN4_bm | PIN6_bm | PIN7_bm); //set pa4, pa6, pa7 as outputs for mosi, sck, and /ss
	VPORTA_OUT |= PIN7_bm; //ss set high initially, disable LCD slave
	SPI0_CTRLA |= (SPI_MASTER_bm | SPI_ENABLE_bm); //enable spi, and make master mode
	SPI0_CTRLB |=  SPI_MODE_0_gc; //set spi mode to 0 
}

//***************************************************************************
//
// Function Name : "init_lcd"
// Target MCU : AVR128DB48
// DESCRIPTION
// Initializes the LCD by setting the cursor to the beginning of the
// display
//
// Inputs : none
//
// Outputs : none
//
//
//**************************************************************************
void init_lcd (void)
{
	init_spi_lcd();		//Initialize mcu for LCD SPI
	_delay_ms(10); //delay 10 ms
	lcd_spi_transmit('|'); //Enter settings mode
	lcd_spi_transmit('-'); //clear display and reset cursor
	
	sprintf(dsp_buff[0], "                    ");
	sprintf(dsp_buff[1], "                    ");
	sprintf(dsp_buff[2], "                    ");
	sprintf(dsp_buff[3], "                    ");
}

//***************************************************************************
//
// Function Name : "clear_lcd"
// Target MCU : AVR128DB48
// DESCRIPTION
// Clears the LCD and sets the cursor to the beginning of the
// display
//
// Inputs : none
//
// Outputs : none
//
//
//**************************************************************************
void clear_lcd (void)
{
	lcd_spi_transmit('|'); //Enter settings mode
	lcd_spi_transmit('-'); //clear display and reset cursor
	
	sprintf(dsp_buff[0], "                    ");
	sprintf(dsp_buff[1], "                    ");
	sprintf(dsp_buff[2], "                    ");
	sprintf(dsp_buff[3], "                    ");
}

void update_lcd(void);
//***************************************************************************
//
// Function Name : "update_lcd"
// Target MCU : AVR128DB48
// DESCRIPTION
// Updates the LCD display by writing the characters from four
// arrays to the LCD using the SPI0 interface
//
// Inputs : none
//
// Outputs : none
//
//
//**************************************************************************
void update_lcd(void)
{

	// send line 1 to the LCD module.
	
	for (int i = 0; i < 20; i++)
	{
		lcd_spi_transmit(dsp_buff[0][i]); //transmit each character of dsp_buff1
	}

	// send line 2 to the LCD module.
	for (int i = 0; i < 20; i++)
	{
		lcd_spi_transmit(dsp_buff[1][i]); //transmit each character of dsp_buff2
	}

	// send line 3 to the LCD module.
	for (int i = 0; i < 20; i++)
	{
		lcd_spi_transmit(dsp_buff[2][i]); //transmit each character of dsp_buff3
	}
	
	// send line 4 to the LCD module.
	for (int i = 0; i < 20; i++)
	{
		lcd_spi_transmit(dsp_buff[3][i]); //transmit each character of dsp_buff4
	}
}

