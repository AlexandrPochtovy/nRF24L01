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


//	module typedef's	---------------------------------------------------------------------------------------------------------
typedef enum nRF24_status_t {//состояние устройства
	nRF_OK,		//устройство подключено
	nRF_Faulth	//устройство неисправно
} nRF24_status;

typedef struct nRF24_MainConfig {
	CONFIG_SET_t config;	//value for CONFIG reg
	uint8_t enaa;			//value for EN_AA reg
	uint8_t enrxaddr;		//value for EN_RXADDR reg
	AddrWidth_t addrWidth;	//value for SETUP_AW reg
	uint8_t sendRetr;		//value for SETUP_RETR reg
	uint8_t sendTimeout;	//value for SETUP_RETR reg
	uint8_t channel;		//value for RF_CH reg
	RF_SET_t rf;			//value for RF_SETUP reg
	uint8_t rx_addr[5];		//value for RX_ADDR_P0 & TX_ADDR regs 
	uint8_t datasize;		//value for RX_PW_P0..RX_PW_P5 regs 
	uint8_t dynpd;			//value for DYNPD reg
	FEATURE_Set_t feat;		//value for FEATURE reg
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
	uint8_t *const txp;
	uint8_t *const rxp;
} nRF24_dev;
//========================		LOW LEVEL		===============================
uint8_t SendShortComm(SPI_Conn *_spi, nRF24_dev *dev, uint8_t cmd, uint8_t value, uint8_t len);

uint8_t SendLongComm(SPI_Conn *_spi, nRF24_dev *dev, uint8_t cmd, uint8_t *data, uint8_t len);

uint8_t ReceiveShortComm(SPI_Conn *_spi, nRF24_dev *dev, uint8_t cmd, uint8_t len);

uint8_t ReceiveLongComm(SPI_Conn *_spi, nRF24_dev *dev, uint8_t cmd, uint8_t *data, uint8_t len);
//========================		MIDDLE LEVEL	================================
uint8_t GetStatus(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t GetConfig(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t SetConfig(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t SetAutoAck(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t SetActiveLines(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t SetAddrWidth(SPI_Conn *_spi, nRF24_dev *dev, AddrWidth_t a_width);

uint8_t SetRetries(SPI_Conn *_spi, nRF24_dev *dev, uint8_t delay, uint8_t count);

uint8_t SetChannel(SPI_Conn *_spi, nRF24_dev *dev, uint8_t channel);

uint8_t SetRF(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t ClearIRQ(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t SetAddrTX(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *addr, uint8_t width);

uint8_t SetAddrRX(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *addr, uint8_t width, Line_t line);

uint8_t SetDataSize(SPI_Conn *_spi, nRF24_dev *dev, Line_t line, uint8_t size);

uint8_t GetStatusFIFO(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t SetDynamicDataLen(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t GetFeature(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t SetFeature(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask);

uint8_t ActivateButt(SPI_Conn *_spi, nRF24_dev *dev);


//========================		HI LEVEL		================================
uint8_t ReadPayload(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, uint8_t len);

uint8_t WritePayload(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, uint8_t len);

uint8_t FlushTX(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t FlushRX(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t RepeatLastTransfer(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t ReadReceiveSize(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t AddDataForAsk(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, uint8_t len, Line_t line);

uint8_t WritePayloadNOASK(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, uint8_t len);

uint8_t SwitchModeTX(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t SwitchModeRX(SPI_Conn *_spi, nRF24_dev *dev);

uint8_t RF24_Init (SPI_Conn *_spi, nRF24_dev *dev, nRF24_MainConfig_t *cfg);

uint8_t RF24_SendData (SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, uint8_t len);
uint8_t RF24_ReceiveData (SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data);
//====================================================================================================
#ifdef __cplusplus
}
#endif
#endif /* MYRF24_ALPHA_NRF24L01_H_ */
