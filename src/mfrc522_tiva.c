/**
 * @file mfrc522_tiva.c
 * @brief MFRC522 MIFARE RFID reader
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date 2020 Mar 6
 */


#include "mfrc522.h"
#include "registers.h"

#include <stdint.h>
#include <stdbool.h>

#include "drivelib/gpio.h"
#include "drivelib/sysctl.h"

#include "spi.h"

static uint32_t SPIBase;
static uint32_t RSTBase;
static uint32_t RSTPin;

static void mfrc522_write(uint8_t register, uint8_t data);
static void mfrc522_writeBuffer(uint8_t reg, const void *buffer, uint16_t size);
static uint8_t mfrc522_read(uint8_t register, uint8_t data);
static void mfrc522_readBuffer(uint8_t reg, void *buffer, uint16_t size);


void tiva_mfrc522_init(uint32_t __SPIBase, uint32_t __RSTBase, uint32_t __RSTPin) {
	SPIBase = __SPIBase;
	RSTBase = __RSTBase;
	RSTPin = __RSTPin;


	// Initialize SPI helper functions
	tiva_spi_open(SPIBase, MODE0);


	// VERY IMPORTANT: reset MFRC522 reader.
	// Read chapter 8.8 MFRC522 Datasheet for detail infomation.
	if (GPIOPinRead(RSTBase, RSTPin) == 0) {
		GPIOPinWrite(RSTBase, RSTPin, RSTPin); // Wake MFRC522 up with hard reset
		SysCtlDelay(50 * SysCtlClockGet() / 3000); // Delay ~50ms
	}
	else { // Using soft reset
		mfrc522_reset();
	}


	// Configurate internal timer
	// f_timer = 40kHz
	mfrc522_write(TModeReg, 0x80);
	mfrc522_write(TPrescalerReg, 0xA9);
	// reload every 50ms
	mfrc522_write(TReloadRegH, 0x07);
	mfrc522_write(TReloadRegL, 0xD0);


	// Configurate general setting for transmitting and receiving
	// Force a 100% ASK
	mfrc522_write(TxASKReg, 0x40);
	// Set CRC preset value to 0x6363, complying to ISO 14443-3 part 6.2.4
	mfrc522_write(ModeReg, 0x3D);
}


void mfrc522_write(uint8_t reg, uint8_t data) {
	// MSB = 0 is Write;
	// Bit 6-1 is Address;
	// LSB always = 0.
	// See chapter 8.1.2 for detail infomation
	// about write operation.
	spi_send((reg << 1) & 0x7E);
	spi_send(data);
}


void mfrc522_writeBuffer(uint8_t reg, const void *buffer, uint16_t size) {
	// MSB = 0 is Write;
	// Bit 6-1 is Address;
	// LSB always = 0.
	// See chapter 8.1.2 for detail infomation
	// about write operation.
	spi_send((reg << 1) & 0x7E);
	spi_sendBuffer(data, size);
}


uint8_t mfrc522_read(uint8_t reg) {
	// MSB = 1 is Read;
	// Bit 6-1 is Address;
	// LSB always = 0.
	// See chapter 8.1.2 for detail infomation
	// about read operation.
	spi_send(((reg << 1) & 0x7E) | 0x80);
	return spi_receive();
}


void mfrc522_readBuffer(uint8_t reg, void *buffer, uint16_t size) {
	// MSB = 1 is Read;
	// Bit 6-1 is Address;
	// LSB always = 0.
	// See chapter 8.1.2 for detail infomation
	// about read operation.
	spi_send(((reg << 1) & 0x7E) | 0x80);
	spi_receiveBuffer(buffer, size);
}


/**************************** End of File ************************************/