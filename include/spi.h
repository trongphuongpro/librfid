/** 
 * @file spi.h
 * @brief Function prototypes for SPI communication protocol
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date 2020 Jan 22
 */

#ifndef __SPI__
#define __SPI__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

/**
 * @brief Device mode: Master or Slave
 */
typedef enum {MASTER=0, SLAVE} SPI_DEVICEMODE;


/**
 * @brief Data mode: mode 0, mode 1, mode 2 and mode 3.
 */
typedef enum {MODE0=0, MODE1, MODE2, MODE3} SPI_DATAMODE;


/**
 * @brief Trasmittion bit order: MSB or LSB
 */
typedef enum {MSB=0, LSB} SPI_BITORDER;


/**
 * @brief Initialize SPI bus for ATmega
 *
 * Default:
 * Master: data mode 0, MSB First.
 * Slave: data mode 0, MSB First, transfer complete interrupt.
 *
 * @param mode MASTER or SLAVE.
 * @return nothing.
 */
void atmega_spi_open(SPI_DEVICEMODE mode);


/**
 * @brief Initialize SPI bus for Tiva C.
 *
 * @param base Memory base of Tiva C SSI module.
 * @param mode MASTER or SLAVE.
 * @return nothing.
 */
void tiva_spi_open(uint32_t base, SPI_DEVICEMODE mode);


/**
 * @brief Set bit order for SPI transmitting.
 * Only for ATmega.
 * @param order MSB or LSB.
 * @return nothing.
 */
void spi_setBitOrder(SPI_BITORDER order);


/**
 * @brief Set data mode for SPI transmitting.
 * Only for ATmega.
 * @param mode MODE0, MODE1, MODE2 or MODE3.
 * @return nothing.
 */
void spi_setDataMode(SPI_DATAMODE mode);


/**
 * @brief Set clock rate prescale. Only for ATmega.
 * 
 * Transmitting speed = MCU's speed / factor.
 * 
 * @param factor 2,4,8,16,32,64,128.
 * @return nothing.
 */
void spi_setPrescaler(uint8_t factor);


/**
 * @brief receive 1 byte from SPI bus. 
 * @return one byte.
 */
uint8_t spi_receive(void);


/**
 * @brief receive 1 array from SPI bus.
 * @param buffer pointer to array.
 * @param len the length of data array.
 * @return nothing.
 */
void spi_receiveBuffer(void *buffer, uint16_t len);


/**
 * @brief send 1 byte to SPI bus.
 * @param data data that will be sent.
 * @return nothing.
 */
void spi_send(uint8_t data);


/**
 * @brief send 1 array to SPI bus.
 * @param buffer pointer to array.
 * @param len the length of data array.
 * @return nothing.
 */
void spi_sendBuffer(const void *buffer, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif /* __SPI__ */