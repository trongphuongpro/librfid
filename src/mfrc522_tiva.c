
//! \file mfrc522_tiva.c
//! \brief MFRC522 MIFARE RFID reader
//! \author Nguyen Trong Phuong (aka trongphuongpro)
//! \date 2020 Mar 6


#include "mfrc522.h"
#include "mfrc522_registers.h"
#include "mfrc522_status.h"

#include <stdlib.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "spi.h"

#define ACTIVATE()		(GPIOPinWrite(SSBase, SSPin, 0))
#define DEACTIVATE()	(GPIOPinWrite(SSBase, SSPin, SSPin))

static uint32_t SPIBase;
static uint32_t SSBase;
static uint32_t SSPin;
static uint32_t RSTBase;
static uint32_t RSTPin;

static void mfrc522_write(uint8_t register, uint8_t data);
static void mfrc522_writeFIFO(const void *buffer, uint16_t size);
static uint8_t mfrc522_read(uint8_t register);
static void mfrc522_readFIFO(void *buffer, uint16_t size);
static void	mfrc522_setRegister(uint8_t reg, uint8_t bits, uint8_t value);
static void mfrc522_softReset();
static void mfrc522_hardReset();
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
											void *crc,
											bool *result);
static uint8_t mfrc522_sendRequestWakeup(uint8_t command);
static uint8_t mfrc522_sendREQA();
uint8_t mfrc522_sendWUPA();
static uint8_t mfrc522_select(uint8_t cmd, uint8_t *buffer, uint8_t *sak_buffer);
static uint8_t mfrc522_anticollision(uint8_t cmd, uint8_t *rxBuffer);


void tiva_mfrc522_init(uint32_t __SPIBase, uint32_t __SSBase, uint32_t __SSPin, uint32_t __RSTBase, uint32_t __RSTPin) {
	SPIBase = __SPIBase;
	SSBase = __SSBase;
	SSPin = __SSPin;
	RSTBase = __RSTBase;
	RSTPin = __RSTPin;

	DEACTIVATE();

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

	mfrc522_hardReset();
	mfrc522_softReset();

	// Configurate internal timer
	// f_timer = 40kHz
	mfrc522_write(TModeReg, 0x80);
	//UARTprintf("TModeReg: %x\n", mfrc522_read(TModeReg));

	mfrc522_write(TPrescalerReg, 0xA9);
	//UARTprintf("TPrescalerReg: %x\n", mfrc522_read(TPrescalerReg));

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


void mfrc522_hardReset() {
	GPIOPinWrite(RSTBase, RSTPin, RSTPin); // Wake MFRC522 up with hard reset
	SysCtlDelay(5 * SysCtlClockGet() / 3000); // Delay ~5ms
}


void mfrc522_softReset() {
	mfrc522_write(CommandReg, MFRC522_CMD_SOFTRESET);
	SysCtlDelay(5 * SysCtlClockGet() / 3000); // Delay ~50ms.

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
	mfrc522_writeFIFO(txBuffer, txSize); // Write data to FIFO

	mfrc522_write(BitFramingReg, bitFraming); //
	mfrc522_write(CommandReg, command);

	// Start the transmission of data
	if (command == MFRC522_CMD_TRANSCEIVE) {
		mfrc522_setRegister(BitFramingReg, BIT_7, BIT_7);
	}

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

	uint8_t errorStatus = mfrc522_read(ErrorReg);

	// Return STATUS_ERROR for [BufferOvfl, ParityErr and ProtocolErr]
	if (errorStatus & 0x13) {
		return STATUS_ERROR;
	}

	// Return STATUS_COLLISION for CollErr
	if (errorStatus & 0x08) {
		return STATUS_COLLISION;
	}

	// Read rx data if user required
	uint8_t __valid_bits;

	if (rxBuffer && rxSize) {

		uint8_t size = mfrc522_read(FIFOLevelReg);
		
		__valid_bits = mfrc522_read(ControlReg) & 0x07; // Read control register for RxLastBits

		if (size > *rxSize) {
			return STATUS_NO_ROOM;
		}

		// Read data from FIFO
		*rxSize = size;
		mfrc522_readFIFO(rxBuffer, *rxSize);

		if (validBits) {
			*validBits = __valid_bits;
		}
	}

	// Check CRC_A validation
	if (rxBuffer && rxSize && checkCRC) {

		// if MIFARE card NAK is not OK
		if (*rxSize == 1 && __valid_bits == 4) {
			return STATUS_MIFARE_NACK;
		}

		// we need at least 2 bytes for CRC_A
		if (*rxSize < 2 || __valid_bits != 0) {
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


uint8_t mfrc522_sendRequestWakeup(uint8_t command) {
	uint16_t ATQA;
	uint8_t ATQA_size = 2;

	mfrc522_setRegister(CollReg, BIT_7, 0); // all received bits will be cleared after a collision

	// using short frame for REQA and WUPA command to RFID card.
	uint8_t validBits = 7;
	uint8_t status = mfrc522_transceive(&command, 1, &ATQA, &ATQA_size, &validBits, false);

	if (status != STATUS_OK) {
		return status;
	}

	// ATQA must be exactly 16 bits.
	if (ATQA_size != 2 || validBits != 0) {
		return STATUS_ERROR;
	}

	return STATUS_OK;
}


uint8_t mfrc522_sendREQA() {
	return mfrc522_sendRequestWakeup(MIFARE_CMD_REQA);
}


uint8_t mfrc522_sendWUPA() {
	return mfrc522_sendRequestWakeup(MIFARE_CMD_WUPA);
}


bool mfrc522_available() {
	return (mfrc522_sendREQA() == STATUS_OK);
}


uint8_t mfrc522_select(uint8_t SEL, uint8_t *buffer, uint8_t *sak_buffer) {
	uint8_t status;
	uint8_t rxSize = 3;
	uint8_t txBuffer[9];

	txBuffer[0] = SEL; // SELECT CASCADE LEVEL n
	txBuffer[1] = 0x70; // NVB (Number of Valid Bits)
	txBuffer[2] = buffer[0];
	txBuffer[3] = buffer[1];
	txBuffer[4] = buffer[2];
	txBuffer[5] = buffer[3];
	txBuffer[6] = buffer[4];

	status = mfrc522_computeAndCheckCRC(txBuffer, 7, txBuffer+7, NULL);

	if (status != STATUS_OK) {
		return status;
	}

	status = mfrc522_transceive(txBuffer, sizeof(txBuffer), sak_buffer, &rxSize, 0, true);

	return status;
}


uint8_t mfrc522_anticollision(uint8_t SEL, uint8_t *rxBuffer) {
	uint8_t status;
	uint8_t rxSize = 5;
	uint8_t txBuffer[7];

	txBuffer[0] = SEL; // ANTICOLLSION CASCADE LEVEL n
	txBuffer[1] = 0x20; // NVB (Number of Valid Bits)

	status = mfrc522_transceive(txBuffer, 2, rxBuffer, &rxSize, NULL, false);

	if (status == STATUS_COLLISION) {
		uint8_t coll_pos = mfrc522_read(CollReg) & 0x1F;
		uint8_t counter = 32; // the maximum number of anticollision loops

		while (counter-- == 0) {

			if (coll_pos == 0) {
				coll_pos = 32;
			}

			txBuffer[0] = SEL;
			txBuffer[1] = 0x20 + coll_pos;
			txBuffer[2] = rxBuffer[0];
			txBuffer[3] = rxBuffer[1];
			txBuffer[4] = rxBuffer[2];
			txBuffer[5] = rxBuffer[3];
			txBuffer[6] = rxBuffer[4];

			rxSize = 5;
			status = mfrc522_transceive(txBuffer, sizeof(txBuffer), rxBuffer, &rxSize, NULL, false);

			if (status == STATUS_COLLISION) {
				status = mfrc522_read(CollReg);
				coll_pos = status & 0x1F;

				// if collision position is not valid
				if (status & BIT_5) {
					return STATUS_COLLISION;
				}
			}
			else {
				return status;
			}
		}
	}

	return status;
}


uint8_t mfrc522_getID(UID_t *uid) {
	uint8_t status = 0;
	uint8_t sak_buffer[3];
	uint8_t uid_buffer[15];
	uint8_t cascade_level = 1;

	mfrc522_setRegister(CollReg, BIT_7, 0); // all received bits will be cleared after a collision

	while (1) {
		switch (cascade_level) {
			case 1:
				status = mfrc522_anticollision(MIFARE_CMD_ANTICOLLCL1, uid_buffer);

				if (status != STATUS_OK) {
					return status;
				}

				status = mfrc522_select(MIFARE_CMD_SELECTCL1, uid_buffer, sak_buffer);

				if (status != STATUS_OK) {
					return status;
				}

				break;

			case 2:
				status = mfrc522_anticollision(MIFARE_CMD_ANTICOLLCL2, uid_buffer+5);

				if (status != STATUS_OK) {
					return status;
				}

				status = mfrc522_select(MIFARE_CMD_SELECTCL2, uid_buffer+5, sak_buffer);

				if (status != STATUS_OK) {
					return status;
				}
				break;

			default:
				return STATUS_INTERNAL_ERROR;
		}

		if (!(sak_buffer[0] & BIT_2)) {
			break;
		}
		else {
			cascade_level++;
		}
	}

	if (cascade_level == 1) {
		uid->UID[0] = uid_buffer[0];
		uid->UID[1] = uid_buffer[1];
		uid->UID[2] = uid_buffer[2];
		uid->UID[3] = uid_buffer[3];
		uid->size = 4;
		uid->SAK = sak_buffer[0];
	}
	else if (cascade_level == 2) {
		uid->UID[0] = uid_buffer[1];
		uid->UID[1] = uid_buffer[2];
		uid->UID[2] = uid_buffer[3];
		uid->UID[3] = uid_buffer[5];
		uid->UID[4] = uid_buffer[6];
		uid->UID[5] = uid_buffer[7];
		uid->UID[6] = uid_buffer[8];
		uid->size = 7;
		uid->SAK = sak_buffer[0];
	}

	return status;
}


uint8_t mfrc522_sendHaltA() {
	uint8_t buffer[4];

	buffer[0] = MIFARE_CMD_HALT;
	buffer[1] = 0x00;

	uint8_t status = mfrc522_computeAndCheckCRC(buffer, 2, buffer+2, NULL);

	if (status != STATUS_OK) {
		return status;
	}

	status = mfrc522_transceive(buffer, 4, NULL, 0, /* txLastBits = */ NULL, /* calc CRC = */ false);

	if (status == STATUS_TIMEOUT) 
		return STATUS_OK;

	if (status == STATUS_OK) 
		return STATUS_ERROR;

	return status;
}


uint8_t mfrc522_computeAndCheckCRC(const void *__buffer, 
									uint8_t size, 
									void *__crc,
									bool *result) {
	uint8_t *buffer = (uint8_t*)__buffer;
	uint8_t *crc = (uint8_t*)__crc;

	mfrc522_write(CommandReg, MFRC522_CMD_IDLE); // cancel current command
	mfrc522_write(DivIrqReg, 0x04); // clear the CRC interrupt bit
	mfrc522_setRegister(FIFOLevelReg, BIT_7, BIT_7); // immediately clear the internal FIFO 
	mfrc522_writeFIFO(buffer, size); // Write data to FIFO
	mfrc522_write(CommandReg, MFRC522_CMD_CALCCRC); // execute command calc CRC

	// waiting for computing CRC
	uint16_t timeout = 1000;
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
	if (result != NULL) {
		if ((buffer[size] == crc[0]) && buffer[size+1] == crc[1]) {
			*result = true;
		}
		else {
			*result = false;
		}
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
	ACTIVATE();
	spi_send((reg << 1) & 0x7E);
	spi_send(data);
	DEACTIVATE();
}


uint8_t mfrc522_read(uint8_t reg) {
	// MSB = 1 is Read;
	// Bit 6-1 is Address;
	// LSB always = 0.
	// See chapter 8.1.2 for detail infomation
	// about read operation.
	ACTIVATE();
	spi_send(((reg << 1) & 0x7E) | 0x80);
	uint8_t data = spi_receive();
	DEACTIVATE();

	return data;
}


void mfrc522_writeFIFO(const void *buffer, uint16_t size) {
	// MSB = 0 is Write;
	// Bit 6-1 is Address;
	// LSB always = 0.
	// See chapter 8.1.2.2 for detail infomation
	// about write operation.
	ACTIVATE();
	spi_send((FIFODataReg << 1) & 0x7E);
	spi_sendBuffer(buffer, size);
	DEACTIVATE();
}


void mfrc522_readFIFO(void *__buffer, uint16_t size) {
	// MSB = 1 is Read;
	// Bit 6-1 is Address;
	// LSB always = 0.
	// See chapter 8.1.2.1 for detail infomation
	// about read operation.

	uint8_t *buffer = (uint8_t*)__buffer;
	uint8_t address = ((FIFODataReg << 1) & 0x7E) | 0x80;

	ACTIVATE();
	spi_send(address);

	for (uint16_t i = 0; i < size-1; i++) {
		buffer[i] = spi_transfer_byte(address);
	}

	// Last byte is receive with 0x00 to STOP receiving.
	buffer[size-1] = spi_transfer_byte(0x00);

	DEACTIVATE();
}


void mfrc522_setRegister(uint8_t reg, uint8_t bits, uint8_t value) {
	uint8_t data = mfrc522_read(reg);

	data = (data & ~bits) | value;
	mfrc522_write(reg, data);
}

/**************************** End of File ************************************/