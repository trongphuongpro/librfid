/* 
 * @file spi_atmega.c
 * @brief Implementation for SPI communication protocol
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date 2020 Jan 22
 */

#include <avr/io.h>
#include "spi.h"

#define SPI_PORT	PORTB
#define SPI_DDR		DDRB

#define MOSI	3
#define MISO	4
#define SCK		5
#define SS		2


static SPI_DEVICEMODE mode;

static void spi_master_open(void);
static void spi_slave_open(void);
static uint8_t spi_master_receive_byte(void);
static uint8_t spi_slave_receive_byte(void);


void spi_master_open(void) {
	mode = MASTER;
	SPI_DDR |= (1 << SCK) | (1 << MOSI);
	SPI_DDR &= ~(1 << MISO);
	SPI_PORT |= (1 << MISO); // pull-up resistor for MISO pin
	
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1); // data mode 0; Prescale = 64
}


void spi_slave_open(void) {
	mode = SLAVE;
	SPI_DDR |= (1 << MISO);
	SPI_DDR &= ~((1 << MOSI) | (1 << SS));
	SPI_PORT |= (1 << MOSI) | (1 << SS);

	SPCR = (1 << SPE) | (1 << SPIE); // enable interrupt
}


void atmega_spi_open(SPI_DEVICEMODE mode) {
	if (mode == MASTER)
		spi_master_open();
	else
		spi_slave_open();
}


void spi_setPrescaler(uint8_t factor) {
	if (mode == MASTER) {
		switch (factor) {
			case 4: case 16: case 64: case 128:	SPSR &= ~(1 << SPI2X);
												break;

			case 2: case 8: case 32:	SPSR |= (1 << SPI2X);
										break;								
		}

		switch (factor) {
			case 2: case 4:	SPCR &= ~(3 << SPR0);
							break;

			case 8: case 16:	SPCR &= ~(3 << SPR0);
								SPCR |= (1 << SPR0);
								break;

			case 32: case 64:	SPCR &= ~(3 << SPR0);
								SPCR |= (2 << SPR0);
								break;

			case 128:	SPCR |= (3 << SPR0);
						break;
		}
	}
}


void spi_setBitOrder(SPI_BITORDER order) {
	SPCR &= ~(1 << DORD);
	SPCR |= (order << DORD);
}


void spi_setDataMode(SPI_DATAMODE mode) {
	SPCR &= ~(3 << CPHA);
	SPCR |= (mode << CPHA);
}


uint8_t spi_transmit_byte(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1 << SPIF))) {
		// wait for flag interrupt
	}
	return SPDR;
}


uint8_t spi_master_receive_byte(void) {
	return spi_transmit_byte(0);
}


uint8_t spi_slave_receive_byte(void) {
	while (!(SPSR & (1 << SPIF))) {
		// wait for flag interrupt
	}
	return SPDR;
}


void spi_send(uint8_t data) {
	spi_transmit_byte(data);
}


void spi_sendBuffer(const void *buffer, uint16_t len) {
	const uint8_t *data = (const uint8_t*)buffer;

	for (uint16_t i = 0; i < len; i++) {
		spi_send(data[i]);
	}
}


uint8_t spi_receive() {
	if (mode == MASTER) {
		return spi_master_receive_byte();
	}
	else {
		return spi_slave_receive_byte();
	}
}


void spi_receiveBuffer(void *buffer, uint16_t len) {
	uint8_t *data = (uint8_t*)buffer;

	for (uint16_t i = 0; i < len; i++) {
		data[i] = spi_receive();
	}
}


/*ISR(SPI_STC_vect) {
	receiveData = SPDR;
}*/

/* End file */