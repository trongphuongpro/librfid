
//! \file mfrc522.h
//! \brief MFRC522 MIFARE RFID reader
//! \author Nguyen Trong Phuong (aka trongphuongpro)
//! \date 2020 Mar 6


#ifndef __RFID__
#define __RFID__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "utils.h"


//! \brief Struct UID_t contains ID array, ID size and SAK of current MIFARE card.
typedef struct __attribute__((packed)) UID {
	uint8_t UID[10]; //!< ID array
	uint8_t size; //!< ID size, 4-byte, 7-byte or 10-byte, depends on card's type.
	uint8_t SAK; //!< 1-byte response from SELECT command.
} UID_t;



//! \brief Initialize MFRC522 Reader for Tiva C MCUs.
//!
//! \param [in] SPIBase Memory base of Tiva C SPI module.
//! \param [in] SSBase Memory base of Slave Select pin.
//! \param [in] SSPin Pin number of Slave Select pin.
//! \param [in] RSTBase Memory base of GPIO pin used for reset MFRC522 reader.
//! \param [in] RSTPin Pin number of GPIO pin used for reset MFRC522 reader.
//!
//! [Note] User need to initialize all of above peripherals before
//! passing memory base to this function.
//!
//! SPI module must be configurated in Mode 0.
//!
//! \return none.
//!
void tiva_mfrc522_init(uint32_t SPIBase, PortPin_t SS, PortPin_t RST);


//! \brief Initialize MFRC522 Reader for ATmega MCUs.
//!
//! \param [in] SSPort Pointer to the port of Slave Select pin.
//! \param [in] SSPin Pin number of Slave Select pin.
//! \param [in] RSTPort Pointer to the port of GPIO pin used for reset MRFC522 reader.
//! \param [in] RSTPin Pin number of GPIO pin used for reset MFRC522 reader.
//!
//! [Note] User need to initialize all of above peripherals before
//! passing pointers to this function. SPI module is configurated inside
//! this function.
//!
//! \return none.
//!
void atmega_mfrc522_init(volatile uint8_t *SSPort, uint8_t SSPin, volatile uint8_t *RSTPort, uint8_t RSTPin);


//! \brief Check if new MIFARE card is avaible
//! \return true or false
//!
bool mfrc522_available();


//! \brief Get card'ID.
//! \param [out] uid Pointer to UID_t instance.
//! \return 0 if success, > 0 if error has occured.
//! 
uint8_t mfrc522_getID(UID_t *uid);


//! \brief Send command HALTA to halt MIFARE card.
//! \return 0 if success, > 0 if error has occured.
//! 
uint8_t mfrc522_sendHaltA();

#ifdef __cplusplus
}
#endif

#endif /* __RFID */

/**************************** End of File ************************************/