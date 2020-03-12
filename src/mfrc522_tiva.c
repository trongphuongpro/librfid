/**
 * @file mfrc522_tiva.c
 * @brief MFRC522 MIFARE RFID reader
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date 2020 Mar 6
 */


#include "mfrc522.h"
#include "mfrc522_registers.h"
#include "mfrc522_status.h"

#include <stdlib.h>

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "inc/hw_memmap.h"

#include "spi.h"

static uint32_t SPIBase;
static uint32_t RSTBase;
static uint32_t RSTPin;

static void mfrc522_write(uint8_t register, uint8_t data);
static void mfrc522_writeBuffer(uint8_t reg, const void *buffer, uint16_t size);
static uint8_t mfrc522_read(uint8_t register);
static void mfrc522_readBuffer(uint8_t reg, void *buffer, uint16_t size);
static void	mfrc522_setRegister(uint8_t reg, uint8_t bits, uint8_t value);
static void mfrc522_reset();
static void mfrc522_enableAntenna();
//static void mfrc522_disableAntenna();
static uint8_t mfrc522_command(uint8_t command,
								uint8_t waitIRq,
								const void *txBuffer,
								uint8_t txSize,
								void *rxBuffer,
								uint8_t *rxSize,
								uint8_t *validBits,
								bool checkCRC);
static uint8_t mfrc522_transceive(const void *txBuffer,
								uint8_t txSize,
								void *rxBuffer,
								uint8_t *rxSize,
								uint8_t *validBits,
								bool checkCRC);
static uint8_t mfrc522_computeAndCheckCRC(const void *rxBuffer, 
											uint8_t rxSize, 
											uint8_t *crc,
											bool *result);
static uint8_t mfrc522_sendRequestWakeup(uint8_t command,
										uint8_t *ATQAbuffer,
										uint8_t size);

void setBitsReg(uint8_t reg, uint8_t mask)
{
	uint8_t temp = mfrc522_read(reg);
	mfrc522_write(reg, temp | mask);
}


void clearBitsReg(uint8_t reg, uint8_t mask)
{
	uint8_t temp = mfrc522_read(reg);
	mfrc522_write(reg, temp & (~mask));
}


void tiva_mfrc522_init(uint32_t __SPIBase, uint32_t __RSTBase, uint32_t __RSTPin) {
	SPIBase = __SPIBase;
	RSTBase = __RSTBase;
	RSTPin = __RSTPin;


	// Initialize SPI helper functions
	tiva_spi_open(SPIBase, MASTER);


	// VERY IMPORTANT: reset MFRC522 reader.
	//Read chapter 8.8 MFRC522 Datasheet for detail infomation.
	// if (GPIOPinRead(RSTBase, RSTPin) == 0) {
	// 	GPIOPinWrite(RSTBase, RSTPin, RSTPin); // Wake MFRC522 up with hard reset
	// 	SysCtlDelay(50 * SysCtlClockGet() / 3000); // Delay ~50ms
	// }
	// else { // Using soft reset
	// 	mfrc522_reset();
	// }

	GPIOPinWrite(RSTBase, RSTPin, RSTPin); // Wake MFRC522 up with hard reset
	SysCtlDelay(50 * SysCtlClockGet() / 3000); // Delay ~50ms
	mfrc522_reset();

	// Configurate internal timer
	// f_timer = 40kHz
	mfrc522_write(TModeReg, 0x80);
	UARTprintf("TModeReg: %x\n", mfrc522_read(TModeReg));

	mfrc522_write(TPrescalerReg, 0xA9);
	UARTprintf("TPrescalerReg: %x\n", mfrc522_read(TPrescalerReg));

	// reload every 50ms
	mfrc522_write(TReloadRegH, 0x07);
	mfrc522_write(TReloadRegL, 0xD0);


	// Configurate general setting for transmitting and receiving
	// Force a 100% ASK
	mfrc522_write(TxASKReg, 0x40);
	// Set CRC preset value to 0x6363, complying to ISO 14443-3 part 6.2.4
	mfrc522_write(ModeReg, 0x3D);


	// Turn antenna on
	mfrc522_enableAntenna();

	//UARTprintf(">> Init done\n");
}


void mfrc522_enableAntenna() {
	// Check if pin TX1 and TX2 are enable or not.
	// If not, turn it on.
	uint8_t status = mfrc522_read(TxControlReg);
	if ((status & 0x03) != 0x03) {
		mfrc522_write(TxControlReg, status | 0x03);
	}
}


// void mfrc522_disableAntenna() {
// 	mfrc522_setRegister(TxControlReg, BIT_0 | BIT_1, 0);
// }


void mfrc522_reset() {
	mfrc522_write(CommandReg, MFRC522_CMD_SOFTRESET);
	SysCtlDelay(50 * SysCtlClockGet() / 3000); // Delay ~50ms.

	while (mfrc522_read(CommandReg) & BIT_4) {
		// While until the PowerDown bit in CommandReg is cleared.
	}
}


uint8_t mfrc522_command(uint8_t command,
						uint8_t waitIRq,
						const void *txBuffer,
						uint8_t txSize,
						void *rxBuffer,
						uint8_t *rxSize,
						uint8_t *validBits,
						bool checkCRC) {

	uint8_t txLastBits = validBits ? *validBits : 0;
	uint8_t bitFraming = txLastBits;

	mfrc522_setRegister(ComIrqReg, BIT_7, 0); // Clear all interrupt request bits
	mfrc522_setRegister(FIFOLevelReg, BIT_7, BIT_7); // immediately clear the internal FIFO 
	mfrc522_write(CommandReg, MFRC522_CMD_IDLE); // Cancel current command execution
	mfrc522_writeBuffer(FIFODataReg, txBuffer, txSize); // Write data to FIFO
	mfrc522_write(BitFramingReg, bitFraming); //
	mfrc522_write(CommandReg, command);

	// Start the transmission of data
	if (command == MFRC522_CMD_TRANSCEIVE) {
		mfrc522_setRegister(BitFramingReg, BIT_7, BIT_7);	
	}

	UARTprintf("CommandReg: %x\n", mfrc522_read(CommandReg));

	// Wait for the command execution to complete.
	// Time-out is 50ms, set in function tiva_mfrc522_init().
	uint8_t irqStatus;

	while (1) {
		irqStatus = mfrc522_read(ComIrqReg); // Read interrupt bits


		if (irqStatus & waitIRq) {
			break;
		}

		if (irqStatus & 0x01) {
			return STATUS_TIMEOUT;
		}
	}

	mfrc522_setRegister(BitFramingReg, BIT_7, 0);

	UARTprintf("irqStatus: %x\n", irqStatus);
	uint8_t errorStatus = mfrc522_read(ErrorReg);
	UARTprintf("errorStatus: %x\n", errorStatus);

	// Return STATUS_ERROR for [BufferOvfl, ParityErr and ProtocolErr]
	if (errorStatus & 0x13) {
		return STATUS_ERROR;
	}

	// Return STATUS_COLLISION for CollErr
	if (errorStatus & 0x08) {
		return STATUS_COLLISION;
	}

	// Read rx data if user required
	uint8_t validBits_;

	if (rxBuffer && rxSize) {
		uint8_t size = mfrc522_read(FIFOLevelReg);
		UARTprintf("size: %d\n", size);
		if (size > *rxSize) {
			return STATUS_NO_ROOM;
		}

		// Read data from FIFO
		*rxSize = size;
		mfrc522_readBuffer(FIFODataReg, rxBuffer, *rxSize);

		// Read control register for RxLastBits
		validBits_ = mfrc522_read(ControlReg) & 0x07;
		UARTprintf("validBits_: %d\n", validBits_);

		if (validBits) {
			*validBits = validBits_;
		}
	}

	// Check CRC_A validation
	if (rxBuffer && rxSize && checkCRC) {
		// if MIFARE card NAK is not OK
		if (*rxSize == 1 && validBits_ == 4) {
			return STATUS_MIFARE_NACK;
		}

		// we need at least 2 bytes for CRC_A
		if (*rxSize < 2 || validBits_ != 0) {
			return STATUS_CRC_WRONG;
		}

		uint8_t crc[2];
		bool result = false;
		uint8_t status = mfrc522_computeAndCheckCRC(rxBuffer, *rxSize - 2, crc, &result);

		if (status != STATUS_OK) {
			return status;
		}

		if (result == false) {
			return STATUS_CRC_WRONG;
		}
	}

	return STATUS_OK;
}


uint8_t mfrc522_transceive(const void *txBuffer,
								uint8_t txSize,
								void *rxBuffer,
								uint8_t *rxSize,
								uint8_t *validBits,
								bool checkCRC) {
	return mfrc522_command(MFRC522_CMD_TRANSCEIVE,
							0x30,
							txBuffer,
							txSize,
							rxBuffer,
							rxSize,
							validBits,
							checkCRC);
}


uint8_t mfrc522_sendRequestWakeup(uint8_t command,
									uint8_t *ATQAbuffer,
									uint8_t size) {
	// check input conditions
	if (ATQAbuffer == NULL || size < 2) {
		return STATUS_NO_ROOM;
	}

	mfrc522_setRegister(CollReg, BIT_7, 0); // all received bits will be cleared after a collision

	// using short frame for REQA and WUPA command to RFID card.
	uint8_t validBits = 7;
	uint8_t status = mfrc522_transceive(&command, 1, ATQAbuffer, &size, &validBits, false);

	if (status != STATUS_OK) {
		return status;
	}

	// ATQA must be exactly 16 bits.
	if (size != 2 || validBits != 0) {
		return STATUS_ERROR;
	}

	return STATUS_OK;
}


uint8_t mfrc522_sendREQA(uint8_t *ATQAbuffer, uint8_t size) {
	return mfrc522_sendRequestWakeup(MIFARE_CMD_REQA, ATQAbuffer, size);
}


uint8_t mfrc522_sendWUPA(uint8_t *ATQAbuffer, uint8_t size) {
	return mfrc522_sendRequestWakeup(MIFARE_CMD_WUPA, ATQAbuffer, size);
}


uint8_t mfrc522_sendHaltA() {
	uint8_t buffer[4];

	buffer[0] = MIFARE_CMD_HALT;
	buffer[1] = 0x00;

	bool tmp = true;
	uint8_t status = mfrc522_computeAndCheckCRC(buffer, 2, buffer+2, &tmp);

	if (status != STATUS_OK) {
		return status;
	}

	status = mfrc522_transceive(buffer, 4, NULL, 0, /* txLastBits = */ 0, /* CRC = */ false);

	if (status == STATUS_TIMEOUT) 
		return STATUS_OK;

	if (status == STATUS_OK) 
		return STATUS_ERROR;

	return status;
}


uint8_t mfrc522_computeAndCheckCRC(const void *rxBuffer_, 
											uint8_t rxSize, 
											uint8_t *crc,
											bool *result) {
	uint8_t *rxBuffer = (uint8_t*)rxBuffer_;

	mfrc522_write(CommandReg, MFRC522_CMD_IDLE); // cancel current command
	mfrc522_write(DivIrqReg, 0x04); // clear the CRC interrupt bit
	mfrc522_setRegister(FIFOLevelReg, BIT_7, BIT_7); // immediately clear the internal FIFO 
	mfrc522_writeBuffer(FIFODataReg, rxBuffer, rxSize); // Write data to FIFO
	mfrc522_write(CommandReg, MFRC522_CMD_CALCCRC); // execute command calc CRC

	// waiting for computing CRC
	uint16_t timeout = 5000;
	uint8_t status;

	while (1) {
		status = mfrc522_read(DivIrqReg);

		// CRC computing done.
		if (status & BIT_2) {
			break;
		}

		if (timeout-- == 0) {
			return STATUS_TIMEOUT;
		}
	}

	// stop computing CRC
	mfrc522_write(CommandReg, MFRC522_CMD_IDLE);

	// get CRC value
	crc[0] = mfrc522_read(CRCResultRegLSB);
	crc[1] = mfrc522_read(CRCResultRegMSB);

	// verify CRC
	if ((rxBuffer[rxSize] == crc[0]) && rxBuffer[rxSize+1] == crc[1]) {
		*result = true;
	}

	return STATUS_OK;
}

/**************************** Helper functions *******************************/

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
	spi_sendBuffer(buffer, size);
}


uint8_t mfrc522_read(uint8_t reg) {
	// MSB = 1 is Read;
	// Bit 6-1 is Address;
	// LSB always = 0.
	// See chapter 8.1.2 for detail infomation
	// about read operation.

	spi_send(((reg << 1) & 0x7E) | 0x80);
	uint8_t data = spi_receive();

	return data;
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


void mfrc522_setRegister(uint8_t reg, uint8_t bits, uint8_t value) {
	uint8_t data = mfrc522_read(reg);

	data = (data & ~bits) | value;
	mfrc522_write(reg, data);
}

/**************************** End of File ************************************/