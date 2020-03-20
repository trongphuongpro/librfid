
//! \file mfrc522_registers.h
//! \brief List of commands and registers of MFRC522 RFID reader.
//! \author Nguyen Trong Phuong (aka trongphuongpro)
//! \date 2020 Mar 6

// User define
#define BIT_0	1
#define BIT_1	2
#define BIT_2	4
#define BIT_3	8
#define BIT_4	16
#define BIT_5	32
#define BIT_6	64
#define BIT_7	128


// MFRC522 Command set
#define	MFRC522_CMD_IDLE          0x00
#define MFRC522_CMD_AUTHENT       0x0E
#define MFRC522_CMD_RECEIVE       0x08             
#define MFRC522_CMD_TRANSMIT      0x04             
#define MFRC522_CMD_TRANSCEIVE    0x0C               
#define MFRC522_CMD_SOFTRESET     0x0F               
#define MFRC522_CMD_CALCCRC       0x03               

// MIFARE Comaand set
#define MIFARE_CMD_REQA           0x26              
#define MIFARE_CMD_WUPA           0x52         
#define MIFARE_CMD_ANTICOLLCL1    0x93   
#define MIFARE_CMD_ANTICOLLCL2    0x95
#define MIFARE_CMD_SELECTCL1      0x93            
#define MIFARE_CMD_SELECTCL2      0x95
#define MIFARE_CMD_AUTHENT1A      0x60            
#define MIFARE_CMD_AUTHENT1B      0x61            
#define MIFARE_CMD_READ           0x30              
#define MIFARE_CMD_WRITE          0xA0              
#define MIFARE_CMD_DECREMENT      0xC0              
#define MIFARE_CMD_INCREMENT      0xC1             
#define MIFARE_CMD_RESTORE        0xC2              
#define MIFARE_CMD_TRANSFER       0xB0              
#define MIFARE_CMD_HALT           0x50         


// MFRC522 registers
// Page 0:Command and Status
#define Reserved00            0x00    
#define CommandReg            0x01    
#define ComIEnReg             0x02    
#define DivIEnReg             0x03    
#define ComIrqReg             0x04    
#define DivIrqReg             0x05
#define ErrorReg              0x06    
#define Status1Reg            0x07    
#define Status2Reg            0x08    
#define FIFODataReg           0x09
#define FIFOLevelReg          0x0A
#define WaterLevelReg         0x0B
#define ControlReg            0x0C
#define BitFramingReg         0x0D
#define CollReg               0x0E
#define Reserved01            0x0F

// Page 1:Communication     
#define Reserved10            0x10
#define ModeReg               0x11
#define TxModeReg             0x12
#define RxModeReg             0x13
#define TxControlReg          0x14
#define TxASKReg              0x15
#define TxSelReg              0x16
#define RxSelReg              0x17
#define RxThresholdReg        0x18
#define DemodReg              0x19
#define Reserved11            0x1A
#define Reserved12            0x1B
#define MifareReg             0x1C
#define Reserved13            0x1D
#define Reserved14            0x1E
#define SerialSpeedReg        0x1F

// Page 2: Configuration    
#define Reserved20            0x20  
#define CRCResultRegMSB       0x21
#define CRCResultRegLSB       0x22
#define Reserved21            0x23
#define ModWidthReg           0x24
#define Reserved22            0x25
#define RFCfgReg              0x26
#define GsNReg                0x27
#define CWGsPReg	          0x28
#define ModGsPReg             0x29
#define TModeReg              0x2A
#define TPrescalerReg         0x2B
#define TReloadRegH           0x2C
#define TReloadRegL           0x2D
#define TCounterValueRegH     0x2E
#define TCounterValueRegL     0x2F

//Page 3: Test     
#define Reserved30            0x30
#define TestSel1Reg           0x31
#define TestSel2Reg           0x32
#define TestPinEnReg          0x33
#define TestPinValueReg       0x34
#define TestBusReg            0x35
#define AutoTestReg           0x36
#define VersionReg            0x37
#define AnalogTestReg         0x38
#define TestDAC1Reg           0x39  
#define TestDAC2Reg           0x3A   
#define TestADCReg            0x3B   
#define Reserved31            0x3C   
#define Reserved32            0x3D   
#define Reserved33            0x3E   
#define Reserved34			  0x3F

/**************************** End of File ************************************/