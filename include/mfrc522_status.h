
//! \file mfrc522_status.h
//! \brief Status when working with MFRC522 RFID reader
//! \author Nguyen Trong Phuong (aka trongphuongpro)
//! \date 2020 Mar 6


// Status set
#define	STATUS_OK				0x00
#define	STATUS_ERROR			0x01
#define	STATUS_COLLISION		0x02
#define	STATUS_TIMEOUT			0x03
#define	STATUS_NO_ROOM			0x04
#define	STATUS_INTERNAL_ERROR	0x05
#define	STATUS_INVALID			0x06
#define	STATUS_CRC_WRONG		0x07
#define	STATUS_MIFARE_NACK		0x08
#define	STATUS_AUTHEN_OK		0x09
#define	STATUS_READ_OK			0x0A
#define	STATUS_WRITE_OK			0x0B
#define	STATUS_CHANGE_OK		0x0C
#define	STATUS_TRANSFER_OK		0x0D
#define	STATUS_STORE_OK			0x0E

/**************************** End of File ************************************/