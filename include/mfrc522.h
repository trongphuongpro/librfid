/**
 * @file mfrc522.h
 * @brief MFRC522 MIFARE RFID reader
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date 2020 Mar 6
 */


#ifndef __RFID__
#define __RFID__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct __attribute__((packed)) UID {
	uint8_t UID[10];
	uint8_t size;
	uint8_t SAK;
} UID_t;


/**
 * @brief Initialize MFRC522 Reader.
 *
 * @param SPIBase Memory base of Tiva C SPI module.
 * @param RSTBase Memory base of GPIO pin used for reset.
 * @param RSTPin Pin number of GPIO pin used for reset.
 *
 * [Note] User need to initialize all of above peripherals before
 * passing memory base to this function.
 *
 * SPI module must be configurated in Mode 0.
 *
 * @return none.
 */
void tiva_mfrc522_init(uint32_t SPIBase, uint32_t SSBase, uint32_t SSPin, uint32_t RSTBase, uint32_t RSTPin);


void atmega_mfrc522_init(volatile uint8_t *SSPort, uint8_t SSPin, volatile uint8_t *RSTPort, uint8_t RSTPin);

bool mfrc522_available();

uint8_t mfrc522_getID(UID_t *uid);

uint8_t mfrc522_sendHaltA();

#ifdef __cplusplus
}
#endif

#endif /* __RFID */

/**************************** End of File ************************************/