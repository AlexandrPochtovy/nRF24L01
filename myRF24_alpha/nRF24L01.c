

#include <myRF24_alpha/nRF24L01.h>

//	some constants and masks	--------------------------------------------------------------------------------------------------
//static const uint8_t lim = 2;
//static const uint8_t RPD_mask = 0x01;// обнаружение частоты сигнала. Если младший бит = 1
/*PLOS_CNT - четыре бита, значение которых увеличивается, вплоть до достижения 15,
при каждой отправке, на которую не получено ожидаемое подтверждение.
Значение сбрасывается при записи в регистр RF_CH.

ARC_CNT - четыре бита, возвращающие количество предпринятых повторов отправки при передаче последнего пакета.
Сбрасывается в 0, когда начинается отправка очередного пакета.
*/
//====	SOME INTERNAL FUNCTION	=================
uint8_t WidthCheck (uint8_t width) {
	return (width >=3) && (width <=5) ? width : 5;
}

uint8_t BufferSizeCheck (uint8_t size) {
	return (size >0) && (size <=32) ? size : 32;
}
//====	LOW LEVEL FUNCTION		=========================================================
uint8_t SendCommand(SPI_Conn *_spi, nRF24_dev *dev, const uint8_t cmd, const uint8_t len) {//write singlebyte command and read hw status
	if (_spi->status == SPI_Bus_Free) {//send setup
		if (_spi->step == 0) {
			_spi->reg = cmd;//send command read one register
			_spi->txp = dev->txp;
			_spi->rxp = dev->rxp;
			_spi->len = len;
			_spi->mode = SPI_MODE_WRITE;
			_spi->step = 1;
			SPI_Start_IRQ_HWNSS(_spi);
		} else if (_spi->step == 1) {
			memcpy(&dev->statusChip, &dev->rxp[0], 1);
			_spi->step = 0;
			return 1;
		} else _spi->step = 0;
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t ReceiveCommand(SPI_Conn *_spi, nRF24_dev *dev, const uint8_t cmd, const uint8_t len) {//write singlebyte command and read hw status
	if (_spi->status == SPI_Bus_Free) {//send setup
		if (_spi->step == 0) {
			_spi->reg = cmd;//send command
			_spi->txp = dev->txp;
			_spi->rxp = dev->rxp;
			_spi->len = len;
			_spi->mode = SPI_MODE_READ;
			_spi->step = 1;
			SPI_Start_IRQ_HWNSS(_spi);
		} else if (_spi->step == 1) {
			memcpy(&dev->statusChip, &dev->rxp[0], 1);
			_spi->step = 0;
			return 1;
		} else _spi->step = 0;
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t WriteMultiReg(SPI_Conn *_spi, nRF24_dev *dev, const uint8_t reg, const uint8_t len) {//write singlebyte command and read hw status
	if (_spi->status == SPI_Bus_Free) {//send setup
		if (_spi->step == 0) {
			_spi->reg = reg | W_REGISTER;//send command read one register
			_spi->txp = dev->txp;
			_spi->rxp = dev->rxp;
			_spi->len = len;
			_spi->mode = SPI_MODE_WRITE;
			_spi->step = 1;
			SPI_Start_IRQ_HWNSS(_spi);
		} else if (_spi->step == 1) {
			memcpy(&dev->statusChip, &dev->rxp[0], 1);
			_spi->step = 0;
			return 1;
		} else _spi->step = 0;
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t ReadMultiReg(SPI_Conn *_spi, nRF24_dev *dev, const uint8_t reg, const uint8_t len) {//write singlebyte command and read hw status
	if (_spi->status == SPI_Bus_Free) {//send setup
		if (_spi->step == 0) {
        _spi->reg = reg | R_REGISTER;//send command read one register
		_spi->txp = dev->txp;
		_spi->rxp = dev->rxp;
        _spi->len = len;
        _spi->mode = SPI_MODE_READ;
        _spi->step = 1;
        SPI_Start_IRQ_HWNSS(_spi);
		} else if (_spi->step == 1) {
			memcpy(&dev->statusChip, &dev->rxp[0], 1);
			_spi->step = 0;
			return 1;
		} else _spi->step = 0;
	}
	return 0;
}
//====	COMMAND FUNCTION		======================
/* читает принятые байты из модуля команда R_RX_PAYLOAD
заодно читает статус нулевым байтом
*/
uint8_t ReadPayload(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len) {
	uint8_t size = BufferSizeCheck(len);
	uint8_t num = GetLineNumber(dev);
	while (!ReceiveCommand(_spi, dev, R_RX_PAYLOAD | num, size)) {}
		memcpy (data, &dev->rxp[1], size);
		return 1;
	//} else return 0;
}
/* записывает байты в модуль для отправки команда W_TX_PAYLOAD
заодно читает статус нулевым байтом
*/
uint8_t WritePayload(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len) {
	//uint8_t size = BufferSizeCheck(len);
	memcpy (dev->txp, data, len);
	while (!SendCommand(_spi, dev, W_TX_PAYLOAD, len)) {}
		return 1;
	//} else return 0;
}
//-----------------------------------------------------
//write one command byte FLUSH_TX and read hw status
//clear TX buffer in chip
uint8_t FlushTX(SPI_Conn *_spi, nRF24_dev *dev) {
	while (!SendCommand(_spi, dev, FLUSH_TX, 0)) {}
	return 1;
	//} else return 0;
}
//-----------------------------------------------------
//write one command byte FLUSH_RX and read hw status
//clear RX buffer in chip
uint8_t FlushRX(SPI_Conn *_spi, nRF24_dev *dev) {
	while (!SendCommand(_spi, dev, FLUSH_RX, 0)) {}
		return 1;
	//} else return 0;
}
//-----------------------------------------------------
//write one command byte REUSE_TX_PL and read hw status
//repeat send last data in TX buffer
uint8_t RepeatLastTransfer(SPI_Conn *_spi, nRF24_dev *dev) {
	while (!SendCommand(_spi, dev, REUSE_TX_PL, 0)) {}
		return 1;
	//} else return 0;
}
//-----------------------------------------------------
//write one command byte R_RX_PL_WID and read hw status
//read RX size data, if > 32 need clear RX buffer
uint8_t ReadReceiveSize(SPI_Conn *_spi, nRF24_dev *dev) {
	while (!ReceiveCommand(_spi, dev, R_RX_PL_WID, 1)) {}
		memcpy(&dev->sizeReceive, &dev->rxp[1], 1);
		return 1;
	//} else return 0;
}
//-----------------------------------------------------
uint8_t AddDataForAsk(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len, const Line_t line) {
	uint8_t size = BufferSizeCheck(len);
	memcpy (dev->txp, data, size);
	while (!SendCommand(_spi, dev, W_ACK_PAYLOAD | line, size)) {}
		return 1;
	//} else return 0;
}
//-----------------------------------------------------
uint8_t WritePayloadNOASK(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len) {
	uint8_t size = BufferSizeCheck(len);
	memcpy (dev->txp, data, size);
	while (!SendCommand(_spi, dev, W_TX_PAYLOAD_NOACK, size)) {}
		return 1;
	//} else return 0;
}
//=======================================================================================================
//		REGISTER FUNCTIONS
//=======================================================================================================
uint8_t GetStatus(SPI_Conn *_spi, nRF24_dev *dev) {
	while (!SendCommand(_spi, dev, RF24_NOP, 0)) {}
		return 1;
	/*} else {
		return 0;
	}*/
}
/*записывает в статус значение напрямую, поменять на более удобную запись числа для сброса флагов прерываний*/
uint8_t SetStatus(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask) {//write singlebyte command and read hw status
	dev->txp[0] = (dev->statusChip & 0x70) | mask;
	while (!WriteMultiReg(_spi, dev, STATUS, 1)) {	}
	return 1;
}
//-----------------------------------------------------
uint8_t SetConfig (SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask) {
	dev->txp[0] = mask;
	while (!WriteMultiReg(_spi, dev, CONFIG, 1)) {}
		return 1;
	//} else return 0;
}

uint8_t ReadConfig (SPI_Conn *_spi, nRF24_dev *dev) {
	while (!ReadMultiReg(_spi, dev, CONFIG, 1)) {}
		return 1;
	//} else return 0;
}
/*добавить отдельные функции включения режима приема/передачи и включение питания*/

//включает функцию подтверждения получения данных на линиях
uint8_t SetAutoAck(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask) {//work
	dev->txp[0]= mask & EN_AA_MASK;
	while (!WriteMultiReg(_spi, dev, EN_AA, 1)) {}
		return 1;
	//} else return 0;
}
//---------------------------------------------------------------
//включает/выкдючает каналы приёмника по маске
uint8_t SetActiveLines(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask) {
	dev->txp[0] = mask & ACTIVE_LINE_MASK;
	while (!WriteMultiReg(_spi, dev, EN_RXADDR, 1)) {}
		return 1;
	//} else return 0;
}
//включает/выкдючает каналы приёмника по маске
uint8_t GetActiveLines(SPI_Conn *_spi, nRF24_dev *dev) {
	while (!ReadMultiReg(_spi, dev, EN_RXADDR, 1)) {}
		return 1;
	//} else return 0;
}
//-------------------------------------------------------------
//work set addr size 3bytes = 0x01, 4bytes = 0x10, 5bytes = 0x11
uint8_t SetAddrWidth(SPI_Conn *_spi, nRF24_dev *dev, AddrWidth_t a_width) {
	dev->txp[0] = a_width;
	while (!WriteMultiReg(_spi, dev, SETUP_AW, 1)){}
		return 1;
	//} else return 0;
}
//задает число попыток и задержку между попытками доставки максимум 15*15
uint8_t SetRetries(SPI_Conn *_spi, nRF24_dev *dev, uint8_t delay, uint8_t count) {
	dev->txp[0] = ((delay << 4) & 0xF0) | (count & 0x0F);
	while (!WriteMultiReg(_spi, dev, SETUP_RETR, 1)){}
		return 1;
	//} else return 0;
}
//----------------------------------------------------------
uint8_t SetChannel(SPI_Conn *_spi, nRF24_dev *dev, uint8_t channel) { //work
	dev->txp[0] = (channel & 0x7F) <= 125 ? channel : 125;
	while (!WriteMultiReg(_spi, dev, RF_CH, 1)){}
		return 1;
	//} else return 0;
}
//---------------------------------------------------------------
uint8_t SetRFReg (SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask) {
	//dev->txp[0] = rf->CONT_WAVE << 7 | rf->PLL_LOCK << 4 | rf->power | rf->speed;
	dev->txp[0] = mask;//число ВЗЯТО ИЗ ПРИМЕРА СТРАННЫЙ БИТ 0
	//dev->txp[0] = rf->power | rf->speed;
	while (!WriteMultiReg(_spi, dev, RF_SETUP, 1)){}
		return 1;
	//} else return 0;
}
//----------------------------------------------------------------
uint8_t ClearIRQ(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask) {//write singlebyte command and read hw status
	dev->txp[0] = (dev->statusChip & 0x70) | mask;
	while (!WriteMultiReg(_spi, dev, STATUS, 1)) {	}
	return 1;
}
//задает адрес передатчика
uint8_t SetAddrTX(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *addr, uint8_t width) {
	uint8_t _width = WidthCheck(width);
	memcpy (dev->txp, addr, _width);
	while (!WriteMultiReg(_spi, dev, TX_ADDR, _width)){}
	return 1;
}
//задает адрес приемника
uint8_t SetAddrRX(SPI_Conn *_spi, nRF24_dev *dev, uint8_t *addr, uint8_t width, const Line_t line) {
	uint8_t _width = WidthCheck(width);
	memcpy (dev->txp, addr, _width);
	while (!WriteMultiReg(_spi, dev, RX_ADDR_P0 + line, _width)){}
		return 1;
	//} else return 0;
}
//задает размер данных при обмене, 1-32 байта
uint8_t SetDataSize(SPI_Conn *_spi, nRF24_dev *dev, const Line_t line, const uint8_t size) {
	dev->txp[0] = BufferSizeCheck(size);
	while (!WriteMultiReg(_spi, dev, RX_PW_P0 + line, 1)){}
		return 1;
	//} else return 0;
}
//-----------------------------------------------------------
uint8_t GetStatusFIFO(SPI_Conn *_spi, nRF24_dev *dev) {
	while (!ReadMultiReg(_spi, dev, FIFO_STATUS, 1)){}
		memcpy(&dev->statusFIFO, &dev->rxp[1], 1);
		return 1;
	//} else return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t SetDynamicDataLen(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask) {
	dev->txp[0] = mask & DYN_MASK;
	while (!WriteMultiReg(_spi, dev, DYNPD, 1)){}
		return 1;
	//} else return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t SetFeature(SPI_Conn *_spi, nRF24_dev *dev, uint8_t mask) {
	dev->txp[0] = mask & 0x03;
	while (!WriteMultiReg(_spi, dev, FEATURE, 1)){}
		return 1;
	//} else return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t GetFeature(SPI_Conn *_spi, nRF24_dev *dev) {
	while (!ReadMultiReg(_spi, dev, FEATURE, 1)){}
		return 1;
	//} else return 0;
}
//------------------------------------------------------------------
uint8_t ActivateButt(SPI_Conn *_spi, nRF24_dev *dev) {
	dev->txp[0] = 0x73;//ВЗЯТО ИЗ ПРИМЕРА
	while (!SendCommand(_spi, dev, ACTIVATE, 1)){}
		return 1;
	//} else return 0;
}
//----------------------------------------------------------------
Line_t GetLineNumber (nRF24_dev *dev) {
	return (dev->statusChip >> 1) & empty;
}
//-------------------------------------------------------------------
void SwitchModeTX(SPI_Conn *_spi, nRF24_dev *dev){
	//прочитать конфиг
	//записать в конфиг бит режима приемника
	//установить в 0 ножку CE
	while (!ReadMultiReg(_spi, dev, CONFIG, 1)) {}
	dev->txp[0] = dev->rxp[1] & 0xFE;
	while (!WriteMultiReg(_spi, dev, CONFIG, 1)) {}
	LL_GPIO_ResetOutputPin(dev->CE_Port, dev->CE_Pin);
}
//---------------------------------------------------------------------------------------------------------------------------------------
void SwitchModeRX(SPI_Conn *_spi, nRF24_dev *dev){
	//прочитать конфиг
	//записать в конфиг бит режима передатчика
	//установить в 1 ножку CEstate
	while (!ReadMultiReg(_spi, dev, CONFIG, 1)) {}
	dev->txp[0] = dev->rxp[1] | 0x01;
	while (WriteMultiReg(_spi, dev, CONFIG, 1) != nRF_Complite) {}
	LL_GPIO_SetOutputPin(dev->CE_Port, dev->CE_Pin);
}
//============================================================================
/*НЕ ЗАБУТЬ БЛЯТЬ ПРО РЕСТАРТ ПОСЛЕДНЕГО ПАКЕТА ПРИ ОШИБКАХ И СЧЕТЧИК ОШИБОК ДОБАВЬ*/
//---------------------------------------------------------------------------------------------------------------------------------------
uint8_t RF24_Init (SPI_Conn *_spi, nRF24_dev *dev, nRF24_MainConfig_t *mainCfg) {
	uint8_t addr[2][5] = { {0xCE, 0xCF, 0xAE, 0xAF, 0xD0}, {0xCE, 0xCF, 0xAE, 0xAF, 0xD1} };
	switch (dev->state) {
		case 0://read status and check connection
			if (GetStatus(_spi, dev)) { dev->state+=1; }
			break;
		case 1://записали число попыток 5 ожидание 15
			if (SetRetries(_spi, dev, 5, 15)) { dev->state+=1; }
			break;
		case 2://запись мощности и скорости 1Mbs 0dB и странный бит 0 (типа включает LNA)
			if (SetRFReg(_spi, dev, 0X07)) { dev->state+=1; }
			break;
		case 3://чтение features
			if (GetFeature(_spi, dev)) { dev->state+=1; }
			break;
		case 4: //недокументированная команда
			if (ActivateButt(_spi, dev)) { dev->state+=1; }
			break;
		case 5:	//чтение features снова
			if (GetFeature(_spi, dev)) { dev->state+=1; }
			break;
		case 6://запись динамического пакета, выключено
			if (SetDynamicDataLen(_spi, dev, 0x00)) { dev->state+=1; }
			break;
		case 7://запись подтверждения каналов все включили
			if (SetAutoAck(_spi, dev, 0x3F)) { dev->state+=1; }
			break;
		case 8://включение каналов активен 0 и 1 каналы
			if (SetActiveLines(_spi, dev, 0x03))  { dev->state+=1; }
			break;
		case 9: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 0, 32)) { dev->state+=1; }
			break;
		case 10: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 1, 32)) { dev->state+=1; }
			break;
		case 11: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 2, 32)) { dev->state+=1; }
			break;
		case 12: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 3, 32)) { dev->state+=1; }
			break;
		case 13: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 4, 32)) { dev->state+=1; }
			break;
		case 14: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 5, 32)) { dev->state+=1; }
			break;
		case 15: //запись ширины адреса 5 байт
			if (SetAddrWidth(_spi, dev, addrWidth_5b))  { dev->state+=1; }
			break;
		case 16://запись радиоканала 76
			if (SetChannel(_spi, dev, 76))  { dev->state+=1; }
			break;
		case 17://запись для стирания всех флагов прерываний
			if (SetStatus(_spi, dev, 0x70)) { dev->state+=1; }
			break;
		case 18://clear buffer RX
			if (FlushRX(_spi, dev)) { dev->state+=1; }
			break;
		case 19://clear buffer TX
			if (FlushTX(_spi, dev)) { dev->state+=1; }
			break;
		case 20://запись конфига питание пока не включили
			if (SetConfig(_spi, dev, 0x0c))   { dev->state+=1; }
			break;
		case 21://чтение конфига
			if (ReadConfig(_spi, dev)) { dev->state+=1; }
			break;
		case 22://запись конфига питание включили
			if (SetConfig(_spi, dev, 0x0e)) { dev->state+=1; }
			break;
		case 23://здесь пауза ппц ждем когда включится модуль
			LL_mDelay(10);
			dev->state+=1;
			break;
		case 24: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 0, 32)) { dev->state+=1; }
			break;
		case 25: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 1, 32)) { dev->state+=1; }
			break;
		case 26: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 2, 32)) { dev->state+=1; }
			break;
		case 27: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 3, 32)) { dev->state+=1; }
			break;
		case 28: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 4, 32)) { dev->state+=1; }
			break;
		case 29: //задаем размер пакета для всех каналов по 32 байта
			if (SetDataSize(_spi, dev, 5, 32)) { dev->state+=1; }
			break;
		case 30:
			if (SetAddrRX(_spi, dev, addr[1], 5, 0)) { dev->state+=1; }
			break;
		case 31:
			if (SetAddrTX(_spi, dev, addr[1], 5)) { dev->state+=1; }
			break;
		case 32:
			if (SetAddrRX(_spi, dev, addr[0], 5, 1)) { dev->state+=1; }
			break;
		case 33://читаем настройки включенных каналов
			if (GetActiveLines(_spi, dev)) { dev->state+=1; }
			break;
		case 34://запись настройки включенных каналов
			if (SetActiveLines(_spi, dev, 0x03)) { dev->state+=1; }
			break;
		case 35://запись включаем питание и режим приемника
			if (SetConfig(_spi, dev, 0x0F)) { dev->state+=1; }
			break;
		case 36://запись для стирания всех флагов прерываний
			if (SetStatus(_spi, dev, 0x70)) { dev->state+=1; }
			break;
		case 37:
			LL_GPIO_SetOutputPin(dev->CE_Port, dev->CE_Pin);
			dev->state+=1;
			break;
		case 38://читаем настройки включенных каналов
			if (GetActiveLines(_spi, dev)) { dev->state+=1; }
			break;
		case 39://запись настройки включенных каналов
			if (SetActiveLines(_spi, dev, 0x02)) {
				dev->state = 0;
				return 1;
			}
			break;
		default:
			dev->state = 0;
			break;
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t RF24_SendData (SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data, const uint8_t len) {
	switch (dev->step) {
		case 0:
			SwitchModeTX(_spi, dev);
			dev->step = 1;
			break;
		case 1://читаем статус модуля и статус фифо
			if (GetStatusFIFO(_spi, dev)) {//смотрим на статус и проверяем что делать
				if (dev->statusFIFO & TX_FIFO_EMPTY) {
					dev->step = 3;
				}
				else {
					__NOP();
					dev->step = 2;
				}
			}
			break;
		case 2:
			if (FlushTX(_spi, dev)) {
				dev->step = 3;
			}
			break;
		case 3:
			if (WritePayload(_spi, dev, data, len)) {
				dev->step = 4;
			}
			break;
		case 4:
				LL_GPIO_SetOutputPin(dev->CE_Port, dev->CE_Pin);
				for (uint32_t i = 0; i < 60; i++) {
					__NOP();
				}
				LL_GPIO_ResetOutputPin(dev->CE_Port, dev->CE_Pin);
				dev->step = 0;
				return 1;
			break;
		default:
			dev->step = 0;
			break;
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t RF24_ReceiveData (SPI_Conn *_spi, nRF24_dev *dev, uint8_t *data) {
	switch (dev->step) {
		case 0://читать статус и статус буфера, затем смотреть что за прерывание пришло
			if (GetStatusFIFO(_spi, dev)) {//смотрим на статус и проверяем что делать
				if ((GetLineNumber(dev) != empty)){
					dev->step = 1;//go read
				}
				else {
					dev->step = 3;//flush & exit
				}
			}
			break;
		case 1://читаем размер принятого пакета и номер линии по которой пришло
			if (ReadReceiveSize(_spi, dev)) { dev->step = 2; }
			break;
		case 2://read data
			if (ReadPayload(_spi, dev, data, dev->sizeReceive)) {dev->step = 0;}
			break;
		case 3://clear IRQ
			if (ClearIRQ(_spi, dev, 0x70)) {
				dev->step = 0; return 1;
			}
			break;
		default:
			dev->step = 0;
			break;
	}
	return 0;
}

