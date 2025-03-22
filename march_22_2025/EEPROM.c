#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

/* Bit value */
#define _BV(bit) (1 << (bit))
// 25LC1024 - Instruction Set
#define EEPROM_READ      0b00000011         // Read memory 
#define EEPROM_WRITE     0b00000010         // Write to memory 
#define EEPROM_WREN      0b00000110         // Write enable 
#define EEPROM_WRDI      0b00000100         // Write disable
#define EEPROM_RDSR      0b00000101         // Read status register 
#define EEPROM_WRSR      0b00000001         // Write status register
#define EEPROM_PE		 0b01000010			// Page erase
#define EEPROM_SE		 0b11011000			// Sector erase

#define EEPROM_RDID		 0b10101011			// Release from deep power
#define EEPROM_DPD		 0b10111001			// Deep power-Down mode

// EEPROM Status Register Bits, use to parse status register
#define EEPROM_WRITE_IN_PROGRESS    0
#define EEPROM_WRITE_ENABLE_LATCH   1
#define EEPROM_BLOCK_PROTECT_0      2
#define EEPROM_BLOCK_PROTECT_1      3

//***************************************************************************
//
// Function Name : "init_spi_EEPROM"
// Target MCU : AVR128DB48
// DESCRIPTION
// Initializes and enables the SPI1 module, disables slave select,
// enables master mode, sets the mode to SPI mode 3, and sets
// the SPI1 GPIO pins to outputs.
// Pseudo code:
//		1. Set the SS pin as output pin and HIGH
//		2. set the MOSI ans SCK pins as outputs
//		3. Set the MISO pin as input and enable pull-up
//		4. Configure settings of SPI module of AVR128DB48
//
// Inputs : none
//
// Outputs : none
//
//
//**************************************************************************
void init_spi_EEPROM(void) 
{	
	/* GPIO configuration settings */
	PORTC.DIR |= PIN4_bm;	// Set PC4 (SS) as output pin
	PORTC.OUT |= PIN4_bm;	// Drive PC4 SS high, de-select EEPROM
	PORTC.DIR |= (PIN0_bm | PIN2_bm);	// Set PC0 (MOSI) & PC2 (SCK) as output pins
	PORTC.DIR &= ~(PIN1_bm);	// Set PC1 (MISO) as input pin
	PORTC.PIN1CTRL |= PORT_PULLUPEN_bm;	// Enable pull-up on PC1 (MOSI)
	
	/* SPI1 configuration settings */	
	SPI1.CTRLA |= (SPI_MASTER_bm | SPI_ENABLE_bm); //enable spi, and make master mode
	SPI1.CTRLB |=  SPI_MODE_0_gc; //set spi mode to 0
}

//***************************************************************************
//
// Function Name : "SPI_tradeByte"
// Target MCU : AVR128DB48
// DESCRIPTION
// Writes data to the SPI data register to initiate a "byte trade"
// Pseudo code:
//		1. Write data to SPI data register
//		2. Wait until SPI_IF bit is set in SPI INT_FLAGS register
//
// Inputs : uint8_t byte : data byte to be sent
//
// Outputs : none
//
//
//**************************************************************************
void SPI_tradeByte(uint8_t byte) 
{	
	SPI1.DATA = byte;	   // SPI starts sending immediately		
	/* Wait until TX is complete, SPI data register now contains received byte*/
	while(!(SPI1.INTFLAGS & SPI_IF_bm)) {}
}

//***************************************************************************
//
// Function Name : "EEPROM_send24BitAddress"
// Target MCU : AVR128DB48
// DESCRIPTION
// Transmits a 24-bit SPI address to the EEPROM
//
// Inputs : uint24_t address : 24-bit address
//
// Outputs : none
//
//**************************************************************************
void EEPROM_send24BitAddress(uint24_t address) 
{
	/* Transmit address bytes one at a a time starting from most significant byte */
	SPI_tradeByte((uint8_t) address.upper);    // Upper address byte
	SPI_tradeByte((uint8_t) address.middle);   // Middle address byte  
	SPI_tradeByte((uint8_t) address.lower);    // Low address byte
}

//***************************************************************************
//
// Function Name : "EEPROM_readStatus"
// Target MCU : AVR128DB48
// DESCRIPTION
// Reads the contents of the status register of the EEPROM
// Pseudo code:
// 	1. Select the EEPROM slave by driving SS LOW
//	2. TX the EEPROM_RDSR OP-code
//	3. TX dummy byte to provide master SCK signal
//	4. Deselect the EEPROM by driving SS HIGH
//	5. Read received data from SPI data register
//
// Inputs : none
//
// Outputs : uint8_t : contents of status register
//
//**************************************************************************
uint8_t EEPROM_readStatus(void) 
{
	PORTC.OUT &= ~PIN4_bm;	// Drive PC4 SS LOW, select EEPROM
	SPI_tradeByte(EEPROM_RDSR);	// RDSR OP-code
	SPI_tradeByte(0);	// dummy byte to initiate SCK
	PORTC.OUT |= PIN4_bm;	// Drive PC4 SS HIGH, de-select EEPROM
	return (SPI1.DATA);	// Received data
}

//***************************************************************************
//
// Function Name : "EEPROM_writeEnable"
// Target MCU : AVR128DB48
// DESCRIPTION
// Enables write access to the EEPROM
// Pseudo code:
//		1. Select the EEPROM slave by driving SS LOW
//		2. TX the EEPROM_WREN OP-code
//		3. Deselect the EEPROM by driving SS HIGH
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void EEPROM_writeEnable(void) 
{
	PORTC.OUT &= ~PIN4_bm;	// Drive PC4 SS LOW, select EEPROM
	SPI_tradeByte(EEPROM_WREN);	// WREN OP-code
	PORTC.OUT |= PIN4_bm;	// Drive PC4 SS HIGH, de-select EEPROM
}

//***************************************************************************
//
// Function Name : "EEPROM_readByte"
// Target MCU : AVR128DB48
// DESCRIPTION
// Reads the 8-bit byte stored in a 24-bit address index of the EEPROM
// Pseudo code:
//		1. Select the EEPROM slave by driving SS LOW
//		2. TX the EEPROM_READ OP-code
//		3. TX 24-bit address
//		4. TX dummy byte to provide master SCK signal
//		5. Deselect the EEPROM by driving SS HIGH
//		6. Read received data from SPI data register
//
// Inputs : uint24_t address : 24-bit address
//
// Outputs : uint8_t : Received data
//
//**************************************************************************
uint8_t EEPROM_readByte(uint24_t address) 
{
	PORTC.OUT &= ~PIN4_bm;	// Drive PC4 SS LOW, select EEPROM
	SPI_tradeByte(EEPROM_READ);	// READ OP-code
	EEPROM_send24BitAddress(address);
	SPI_tradeByte(0);	// dummy byte for master SCK signal
	PORTC.OUT |= PIN4_bm;	// Drive PC4 SS HIGH, de-select EEPROM
	return (SPI1.DATA);	// Received data
}

//***************************************************************************
//
// Function Name : "EEPROM_readWord"
// Target MCU : AVR128DB48
// DESCRIPTION
// Reads the 16-bit word stored in a 24-bit address index of the EEPROM
// Pseudo code:
//		1. Select the EEPROM slave by driving SS LOW
//		2. TX the EEPROM_READ OP-code
//		3. TX 24-bit address
//		4. TX dummy byte to read high byte of word
//		5. Read received data from SPI data register
//		6. TX another dummy byte to read low byte of word
//		7. Read received data from SPI data register
//		8. Deselect the EEPROM by driving SS HIGH
//
// Inputs : uint24_t address : 24-bit address
//
// Outputs : uint16_t : Received data
//
//**************************************************************************
uint16_t EEPROM_readWord(uint24_t address) 
{
	uint16_t eepromWord;
	PORTC.OUT &= ~PIN4_bm;	// Drive PC4 SS LOW, select EEPROM
	SPI_tradeByte(EEPROM_READ);	// READ OP-code
	EEPROM_send24BitAddress(address);
	SPI_tradeByte(0);	// dummy byte for master SCK signal
	eepromWord = SPI1.DATA;
	eepromWord = eepromWord << 8;	// high-byte of word
	SPI_tradeByte(0);
	eepromWord += SPI1.DATA;	// low-byte of word 
	PORTC.OUT |= PIN4_bm;	// Drive PC4 SS HIGH, de-select EEPROM
	return (eepromWord);	// Received data  
}

//***************************************************************************
//
// Function Name : "EEPROM_writeByte"
// Target MCU : AVR128DB48
// DESCRIPTION
// Writes an 8-bit byte to a 24-bit address index of the EEPROM
// Pseudo code:
//		1. Enable write-access to EEPROM
//		2. Select the EEPROM slave by driving SS LOW
//		3. TX the EEPROM_WRITE OP-code
//		4. TX 24-bit address
//		5. TX write data
//		6. Deselect the EEPROM by driving SS HIGH
//		7. Wait until write transaction is complete
//
// Inputs : uint24_t address : 24-bit address
//
// Outputs : none
//
//**************************************************************************
void EEPROM_writeByte(uint24_t address) 
{
	EEPROM_writeEnable();	// enable write-access
	PORTC.OUT &= ~PIN4_bm;	// Drive PC4 SS LOW, select EEPROM
	SPI_tradeByte(EEPROM_WRITE);	// WRITE OP-code
	EEPROM_send24BitAddress(address);
	SPI_tradeByte(0);
	PORTC.OUT |= PIN4_bm;	// Drive PC4 SS HIGH, de-select EEPROM
	
  	/* Wait until transaction is complete */
	while (EEPROM_readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {}
}

//***************************************************************************
//
// Function Name : "EEPROM_writeWord"
// Target MCU : AVR128DB48
// DESCRIPTION
// Writes an 16-bit word to a 24-bit address index of the EEPROM
// Pseudo code:
//		1. Enable write-access to EEPROM
//		2. Select the EEPROM slave by driving SS LOW
//		3. TX the EEPROM_WRITE OP-code
//		4. TX 24-bit address
//		5. TX high byte of data
//		6. TX low byte of data
//		7. Deselect the EEPROM by driving SS HIGH
//		8. Wait until write transaction is complete
//
// Inputs : uint24_t address : 24-bit address
//			uint16_t word	 : Data word to be sent
//
// Outputs : none
//
//**************************************************************************
void EEPROM_writeWord(uint24_t address, uint16_t word)
{
	EEPROM_writeEnable();	// enable write-access
	PORTC.OUT &= ~PIN4_bm;	// Drive PC4 SS LOW, select EEPROM
	SPI_tradeByte(EEPROM_WRITE);	// WRITE OP-code
	EEPROM_send24BitAddress(address);
	SPI_tradeByte((uint8_t) (word >> 8));	// high-byte of word
    SPI_tradeByte((uint8_t) word);	// low-byte of word
	PORTC.OUT |= PIN4_bm;	// Drive PC4 SS HIGH, de-select EEPROM
	
	/* Wait until transaction is complete */
	while (EEPROM_readStatus() & _BV(EEPROM_WRITE_IN_PROGRESS)) {}
}