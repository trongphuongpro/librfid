/** 
 * @file spi_tiva.c
 * @brief Wrapper for SPI communication protocol of Tiva C.
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date 2020 Mar 2
 */


#include "spi.h"

#include <stdbool.h>
#include "driverlib/ssi.h"


static uint32_t SSIBase;
static SPI_DEVICEMODE mode;


void tiva_spi_open(uint32_t base, SPI_DEVICEMODE __mode) {
	SSIBase = base;
	mode = __mode;

	SSIEnable(SSIBase);
}


uint8_t spi_transfer_byte(uint8_t data) {
	SSIDataPut(SSIBase, data);

	while (SSIBusy(SSIBase)) {

	}

	uint32_t dump;
	SSIDataGet(SSIBase, &dump);

	return dump;
}


uint8_t spi_master_receive_byte(void) {
	return spi_transfer_byte(0xFF);
}


uint8_t spi_slave_receive_byte(void) {
	uint32_t data;

	SSIDataGet(SSIBase, &data);

	return data;
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

/**************************** End of File ************************************/