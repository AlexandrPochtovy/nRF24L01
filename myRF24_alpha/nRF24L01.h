/*
 * nRF24L01.h
 *
 *  Created on: 23 февр. 2022 г.
 *      Author: alexm
 */
#ifndef MYRF24_ALPHA_NRF24L01_H_
#define MYRF24_ALPHA_NRF24L01_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "string.h"
#include "RF24Register.h"
#include "MySPI.h"

//===================================================================
//----	COMMON TYPEDEF's	-------------------------------------------------------------------------------------
typedef enum ON_OFF {
	ON_0 	= 0x00,
	OFF_1 = 0x01
} ON_OFF_t;

typedef enum OFF_ON {
	OFF_0 = 0x00,
	ON_1 	= 0x01
} OFF_ON_t;

//----	CONFIG setup		-------------------------------------------------------------------------------------
typedef enum nRF_CRC {
	crc_0 = 0x00,//RW Включает CRC 1 - ON; 0 - OFF
	crc_1 = 0x08,
	crc_2 = 0x0C
} nRF_CRC_t;

typedef struct ConfigSetup {
	ON_OFF_t MASK_RX_DR;	//6 bit RW включает прерывание по RX_DR (получение пакета) 1 - OFF | 0 - EN
	ON_OFF_t MASK_TX_DS;	//5 bit RW включает прерывание по TX_DS (отправка пакета или получение подтверждения о доставке) 1-OFF | 0-EN
	ON_OFF_t MASK_MAX_RT;	//4 bit RW включает прерывание по MAX_RT (превышение числа повторных попыток отправки) 1-OFF | 0-EN
	nRF_CRC_t crc; 					//3&2 bits RW Размер поля CRC: 0 - 1 байт; 1 - 2 байта
	OFF_ON_t PWR_UP;			//1 bit RW Включение питания 1 - Power Up; 0 - Power Down, помни про временные задержки на включение
	uint8_t PRIM_RX;			//0 bit RW Выбор режима: 0 - TX (передатчик); 1 - RX (приёмник)
} ConfigSetup_t;

typedef enum AddrWidth {
	addrWidth_3b = 0x01,
	addrWidth_4b = 0x02,
	addrWidth_5b = 0x03
} AddrWidth_t;
//----	RF setup		-------------------------------------------------------------------------------------
typedef enum RF_Speed {
	nRF_1Mbps		= 0x00,	//RF_DR_LOW = 0 & RF_DR_HIGH = 0
	nRF_2Mbps		= 0x08,	//RF_DR_LOW = 0 & RF_DR_HIGH = 1
	nRF_250kbps	=	0x20	//RF_DR_LOW = 1 & RF_DR_HIGH = 0
} RF_Speed_t;
typedef enum RF_Pwr {
	RF_PWR_m18db	= 0x00,	//2бита мощность: 00 -18dBm;
	RF_PWR_m12db	= 0x02,	//2бита мощность: 01 -16dBm;
	RF_PWR_m6db		= 0x04,	//2бита мощность: 10 -6dBm;
	RF_PWR_0db		= 0x06	//2бита мощность: 11  0dBm
} RF_PWR_t;

typedef struct RF_SETUP {
	//uint8_t CONT_WAVE; //7 bit RW (Только для nRF24L01+) 1 - непрерывная передача несущей (для тестов), 0 - OFF
	RF_Speed_t speed;
	//uint8_t PLL_LOCK; //4 bit RW Для тестов
	RF_PWR_t power;
} RF_SETUP_t;

typedef struct FEATURE_Bits {
	OFF_ON_t EN_DPL; 		//RW Включает поддержку приёма и передачи пакетов произвольной длины
	OFF_ON_t EN_ACK_PAY;//RW Разрешает передачу данных с пакетами подтверждения приёма
	OFF_ON_t EN_DYN_ACK;//RW Разрешает использование W_TX_PAYLOAD_NOACK (отправка пакета без подтверждения)
} FEATURE_Bits_t;

typedef enum Line {
	line0 = 0x00,
	line1 = 0x01,
	line2 = 0x02,
	line3 = 0x03,
	line4 = 0x04,
	line5 = 0x05,
	empty = 0x07
} Line_t;

typedef enum IRQ_bit {
	RX_DR		= 0x40,	//RW
	TX_DS		= 0x20,	//RW
	MAX_RT	= 0x10,	//RW
} IRQ_bit_t;
//	module typedef's	---------------------------------------------------------------------------------------------------------
typedef enum nRF24_status_t {//состояние устройства
	nRF_Init,		//устройство не настроено
	nRF_OK,		//устройство готово к опросу
	nRF_Faulth	//устройство неисправно
} nRF24_status;

/*	состояние обмена данными с устройством, использовать для завершения функции работы с устройством */
typedef enum nRF24_Connect_t {
	nRF_Processing, //выполняется работа с устройством: обмен данными, обработка результатов
	nRF_Complite	//работа с устройством завершена, данные считаны/записаны корректно
} nR24_Connect;

typedef struct nRF24_MainConfig {
	ConfigSetup_t cfg;//CONFIG REG
	uint8_t enaa;
	uint8_t enRXline;
	AddrWidth_t addrWidth;
	uint8_t sendRetr;
	uint8_t sendTimeout;
	uint8_t channel;
	RF_SETUP_t rf;
	uint8_t addr_p0[5];
	uint8_t addr_p1[5];
	uint8_t addr_p2[5];
	uint8_t addr_p3[5];
	uint8_t addr_p4[5];
	uint8_t addr_p5[5];
	uint8_t lenght;
	uint8_t dynpl;
	FEATURE_Bits_t feat;
} nRF24_MainConfig_t;
//common data struct for module
typedef struct nRF24_dev_t {
	GPIO_TypeDef *CE_Port;
	uint32_t CE_Pin;
	nRF24_status devStatus;
	uint8_t step;	//step for internal processing
	uint8_t state;	//step for procedure processing
	uint8_t statusChip;
	uint8_t statusFIFO;
	Line_t lineNumber;
	uint8_t sizeReceive;
	uint8_t *const txp;
	uint8_t *const rxp;
} nRF24_dev;
//====	SOME INTERNAL FUNCTION	=================

//========================		LOW LEVEL		===============================
uint8_t SendCommand(SPI_Conn *_spi, nRF24_dev *dev, const uint8_t cmd, const uint8_t len);

uint8_t ReceiveCommand(SPI_Conn *_spi, nRF24_dev *dev, const uint8_t cmd, const uint8_t len);

uint8_t WriteMultiReg(SPI_Conn *_spi, nRF24_dev *dev, const uint8_t reg, const uint8_t len);

uint8_t ReadMultiReg(SPI_Conn *_spi, nRF24_dev *dev, const uint8_t reg, const uint8_t len);

//========================		HI LEVEL			================================
uint8_t ReadPayload(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len);

uint8_t WritePayload(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len);

uint8_t FlushTX(SPI_Conn *_spi, nRF24_dev *dev);//ok

uint8_t FlushRX(SPI_Conn *_spi, nRF24_dev *dev);//ok

uint8_t RepeatLastTransfer(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t ReadReceiveSize(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t AddDataForAsk(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len, const Line_t line);

uint8_t WritePayloadNOASK(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len);
//------------------------------------------------------------------------------------------------------------
uint8_t GetStatus(SPI_Conn *_spi, nRF24_dev *dev);//ok

uint8_t SetConfig (SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);//ok

uint8_t ReadConfig (SPI_Conn *_spi, nRF24_dev *dev);

uint8_t SetAutoAck(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);//ok

uint8_t SetActiveLines(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);//ok

uint8_t GetActiveLines(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t SetAddrWidth(SPI_Conn *_spi, nRF24_dev *dev, AddrWidth_t a_width);//ok

uint8_t SetRetries(SPI_Conn *_spi, nRF24_dev *dev, uint8_t delay, uint8_t count);//ok

uint8_t SetChannel(SPI_Conn *_spi, nRF24_dev *dev, uint8_t channel);//ok

uint8_t SetRFReg (SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);//ok

uint8_t ClearIRQ(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t SetAddrTX(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *addr, uint8_t width);//ok

uint8_t SetAddrRX(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *addr, uint8_t width, const Line_t line);//ok

uint8_t SetDataSize(SPI_Conn *_spi, nRF24_dev *dev, const Line_t line, const uint8_t size);

uint8_t GetStatusFIFO(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t SetDynamicDataLen(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t SetFeature(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t GetFeature(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t ActivateButt(SPI_Conn *_spi, nRF24_dev *dev);

Line_t GetLineNumber (nRF24_dev *dev);

void SwitchModeTX(SPI_Conn *_spi, nRF24_dev *dev);

void SwitchModeRX(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t RF24_Init (SPI_Conn *_spi, nRF24_dev *dev, nRF24_MainConfig_t *mainCfg);

uint8_t RF24_SendData (SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len);
uint8_t RF24_ReceiveData (SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data);
//====================================================================================================
#ifdef __cplusplus
}
#endif
#endif /* MYRF24_ALPHA_NRF24L01_H_ */
