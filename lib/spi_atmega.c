/** 
 * @file spi_atmega.c
 * @brief Implementation for SPI communication protocol
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date 2020 Jan 22
 */


#include "spi.h"

#include <avr/io.h>


static uint8_t mode;

static uint8_t spi_master_receive_byte(void);
static uint8_t spi_slave_receive_byte(void);


void atmega_spi_master_init(uint8_t data_mode, uint8_t prescale) {
	mode = MASTER;
	ATMEGA_SPI_DDR |= (1 << ATMEGA_SCK) | (1 << ATMEGA_MOSI);
	ATMEGA_SPI_DDR &= ~(1 << ATMEGA_MISO);
	ATMEGA_SPI_PORT |= (1 << ATMEGA_MISO); // pull-up resistor for MISO pin
	
	SPCR = (1 << SPE) | (1 << MSTR); // data mode 0; Prescale = 64

	atmega_spi_setDataMode(data_mode);
	atmega_spi_setBitOrder(MSB);
	atmega_spi_setPrescaler(prescale);
}


void atmega_spi_slave_init(uint8_t data_mode) {
	mode = SLAVE;
	ATMEGA_SPI_DDR |= (1 << ATMEGA_MISO);
	ATMEGA_SPI_DDR &= ~((1 << ATMEGA_MOSI) | (1 << ATMEGA_SS));
	ATMEGA_SPI_PORT |= (1 << ATMEGA_MOSI) | (1 << ATMEGA_SS);

	SPCR = (1 << SPE) | (1 << SPIE); // enable interrupt

	atmega_spi_setDataMode(data_mode);
	atmega_spi_setBitOrder(MSB);

}


void atmega_spi_setPrescaler(uint8_t factor) {
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


void atmega_spi_setBitOrder(uint8_t order) {
	SPCR &= ~(1 << DORD);
	SPCR |= (order << DORD);
}


void atmega_spi_setDataMode(uint8_t mode) {
	SPCR &= ~(3 << CPHA);
	SPCR |= (mode << CPHA);
}


uint8_t spi_transfer_byte(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1 << SPIF))) {
		// wait for flag interrupt
	}
	return SPDR;
}


uint8_t spi_master_receive_byte(void) {
	return spi_transfer_byte(0xFF);
}


uint8_t spi_slave_receive_byte(void) {
	while (!(SPSR & (1 << SPIF))) {
		// wait for flag interrupt
	}
	return SPDR;
}


void spi_send(uint8_t data) {
	spi_transfer_byte(data);
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


/*ISR(ATMEGA_SPI_STC_vect) {
	receiveData = SPDR;
}*/

/**************************** End of File ************************************/