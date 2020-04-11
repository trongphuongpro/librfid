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
#include <stdbool.h>


#define ATMEGA_SPI_PORT	PORTB
#define ATMEGA_SPI_DDR	DDRB

#define ATMEGA_MOSI	3
#define ATMEGA_MISO	4
#define ATMEGA_SCK	5
#define ATMEGA_SS	2


#define TIVA_SPI_MODE0  0
#define TIVA_SPI_MODE1  2
#define TIVA_SPI_MODE2  1
#define TIVA_SPI_MODE3  3

#define ATMEGA_SPI_MODE0  0
#define ATMEGA_SPI_MODE1  1
#define ATMEGA_SPI_MODE2  2
#define ATMEGA_SPI_MODE3  3

#define MASTER  0
#define SLAVE   1

#define MSB     0
#define LSB     1


void atmega_spi_master_init(uint8_t data_mode, uint8_t prescale);
void atmega_spi_slave_init(uint8_t data_mode);


/**
 * @brief Initialize SPI bus for Tiva C.
 *
 * @param base Memory base of Tiva C SSI module.
 * @param mode MASTER or SLAVE.
 * @return nothing.
 */
void tiva_spi_master_init(uint32_t base, 
                            uint32_t data_mode, 
                            uint32_t speed, 
                            uint8_t data_width);


void tiva_spi_slave_init(uint32_t base, uint32_t data_mode, uint8_t data_width);



/**
 * @brief Set bit order for SPI transferting.
 * Only for ATmega.
 * @param order MSB or LSB.
 * @return nothing.
 */
void atmega_spi_setBitOrder(uint8_t order);


/**
 * @brief Set data mode for SPI transferting.
 * Only for ATmega.
 * @param mode MODE0, MODE1, MODE2 or MODE3.
 * @return nothing.
 */
void atmega_spi_setDataMode(uint8_t mode);


/**
 * @brief Set clock rate prescale. Only for ATmega.
 * 
 * Transmitting speed = MCU's speed / factor.
 * 
 * @param factor 2,4,8,16,32,64,128.
 * @return nothing.
 */
void atmega_spi_setPrescaler(uint8_t factor);


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


/**
 * @brief transfer 1 byte to SPI bus.
 * @param data 1-byte data.
 * @return 1-byte received data.
 */
uint8_t spi_transfer_byte(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* __SPI__ */

/**************************** End of File ************************************/