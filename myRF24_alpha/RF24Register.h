/* адресация каналов *********************************************************************************************/
/*40-битный (5 байт) регистр RX_ADDR_P0 используемый для указания адреса канала 0 приёмника.
Этот канал используется для приёма автоподтверждений в режиме передатчика.
Автоподтверждения высылаются принимающей стороной с указанием собственного адреса.
Поэтому значение этого регистра должно соответствовать значению регистра TX_ADDR для корректной работы в режиме передатчика.
Реальная используемая ширина адреса задаётся в регистре SETUP_AW.
Значение регистра записывается и читается, начиная с младших байт. Если записано менее 5 байт, то старшие байты остаются неизменными.
Значение регистра по умолчанию: 0xE7E7E7E7E7
40-битный (5 байт) регистр RX_ADDR_P1 используется для указания адреса канала 1 приёмника.
Старшие 4 байта этого регистра являются общими для адресов на каналах 1 - 5.
Реальная используемая ширина адреса задаётся в регистре SETUP_AW.
Значение регистра записывается и читается, начиная с младших байт. Если записано менее 5 байт, то старшие байты остаются неизменными.
Значение регистра по умолчанию: 0xC2C2C2C2C2
8-битные регистры RX_ADDR_P2..RX_ADDR_P5 задают значения младшего байта адреса для каналов 2-5.
Значения старших 32 бит берутся из регистра RX_ADDR_P1.
Значение регистров по умолчанию: 0xC3, 0xC4, 0xC5, 0xC6, соответственно.

40-битный (5 байт) регистр TX_ADDR, используемый в режиме передатчика в качестве адреса удалённого устройства.
При включенном режиме автоподтверждения, удалённое устройство ответит подтверждением с указанием своего же адреса.
Это подтверждение принимается на канале 0, поэтому для успешной передачи, значение регистра RX_ADDR_P0 должно быть идентично этому.
Реальная используемая ширина адреса задаётся в регистре SETUP_AW.
Значение регистра записывается и читается, начиная с младших байт. Если записано менее 5 байт, то старшие байты остаются неизменными.
Значение регистра по умолчанию: 0xE7E7E7E7E7*/

//ARD - четыре бита, задающие время ожидания подтверждения приёма перед повторной отправкой:
//0000 - 250мкс; 0001 - 500мкс; 0010 - 750мкс; ... ; 1111 - 4000мкс;
//Значение поля ARD необходимо выбирать в зависимости от скорости обмена и количества данных,
//передаваемых с пакетом подтверждения. Если данные с пакетом подтверждения не передаются,
//то для скоростей обмена 1 и 2 Мбит/с достаточное время ожидания 250мкс.
//Если вместе с пакетом подтверждения на скорости 1Мбит/с передаётся более 5 байт данных,
//или более 15 на скорости 2Мбит/с, или скорость обмена выбрана 250кбит/с, то необходимо выбрать паузу 500мкс.
//ARC - четыре бита, задающие количество автоматических повторов отправки.
//0000 - автоповтор отключен; 0001 - возможен однократный повтор; ... ; 1111 - возможно до 15 повторов.


#define EN_AA_MASK 			0x3F
#define ACTIVE_LINE_MASK 	0x3F
#define DYN_MASK 			0x3F

//=========================================  команды радиомодуля  ============================================
//команда начинается с отправки командного байта, и потом с отправки байта(ов) данных
//любая команда всегда возвращает статус радиомодуля, можно использовать для проверки подключенного радиомодуля
enum Command {
	R_REGISTER			= 0x00,	//use (R_REGISTER | ADDR) Read command and status registers. ADDR = 5 bit Register Map Address
	W_REGISTER		= 0x20,	//use (W_REGISTER | ADDR) Write command and status registers. ADDR = 5 bit Register Map Address 
												//Executable in power down or standby modes only.
	R_RX_PAYLOAD 	= 0x61,	//Read RX-payload: 1 – 32 bytes. A read operation always starts at byte 0. Payload is deleted from
												//FIFO after it is read. Used in RX mode
	W_TX_PAYLOAD	= 0xA0, //Записать в буфер передатчика данные для отправки
	FLUSH_TX			= 0xE1,	//Сбросить буфер передатчика, работает только в режиме передатчика
	FLUSH_RX			= 0xE2,	//Сбросить буфер приёмника, работает только в режиме приемника
	REUSE_TX_PL		= 0xE3,	//повторно отправить последний пакет
	R_RX_PL_WID		= 0x60,	//Считать размер буфера принятого пакета приёмника. Если больше 32 - ошибка, нужно очистить командой FLUSH_RX
	W_ACK_PAYLOAD	= 0xA8,	// + p Записать автоответ для канала p. При этом бит EN_ACK_PAY в регистре FEATURE должен быть установлен
													//команда для канала - W_ACK_PAYLOAD & number (10101PPP), ppp - number 000..101 разрешенные
	W_TX_PAYLOAD_NOACK = 0xB0,	//Записать в передатчик данные, которые отправляются без ожидания автоответа
	RF24_NOP			= 0xFF,	//Нет операции. Может быть использовано для чтения регистра статуса или для поверки подключенного модуля
	ACTIVATE			= 0x50,	//команда может понадобиться для китайского клона радиомодуля
};
//=========================================  регистры радиомодуля  ===========================================
enum Registers {
	/*7		|			6				|			5				|			4					|		3			|		2		|		1			|		0
		res0	|	MASK_RX_DR	|	MASK_TX_DS	|	MASK_MAX_RT	|	EN_CRC	|	CRCO	|	PWR_UP	|	PRIM_RX*/
	CONFIG 			= 0x00,	//RW настройка прерываний, контрольной суммы, питания и статуса TX/RX
	//-----------------------------------------------------------------------------------------------------------------------------------
	/*7			|	6			|		5			|		4			|		3			|		2			|		1			|		0
	//res0	|	res0	|	ENAA_P5	|	ENAA_P4	|	ENAA_P3	|	ENAA_P2	|	ENAA_P1	|	ENAA_P0*/
	EN_AA				= 0x01,	//RW включает автоподтверждение приема на выбранном канале RX 1 = ON; 0 = OFF
	//-----------------------------------------------------------------------------------------------------------------------------------
	/*	7		|	6		|		5		|		4		|		3		|		2		|		1		|		0
		res0	|	res0	|	ERX_P5	|	ERX_P4	|	ERX_P3	|	ERX_P2	|	ERX_P1	|	ERX_P0
	 * RW При использовании устройства в качестве передатчика с включенной функцией автоподтверждения
	ответ от удалённого устройства принимается на канале 0.
	Поэтому бит ERX_P0 должен быть установлен в 1, для использования передачи с автоподтвереждением*/
	EN_RXADDR	= 0x02,	//RW Выбор каналов приёмника, включение и отключение канала Rx
	//-----------------------------------------------------------------------------------------------------------------------------------
	SETUP_AW	= 0x03,	//RW Задаёт длину поля адреса
	//-----------------------------------------------------------------------------------------------------------------------------------
	SETUP_RETR 	= 0x04,	//RW Настройка повторной отправки пакетов при ошибках
	//-----------------------------------------------------------------------------------------------------------------------------------
	RF_CH			= 0x05,	//RW Регистр задаёт номер радиоканала - частоту несущей с шагом 1Мгц.
											//Радиочастота несущей вычисляется по формуле 2400 + RF_CH МГц.
											//Допустимые значения от 0 до 125. При обмене на скорости 2Мбит/с,
											//частота должна отличатся от частоты используемой другими устройствами минимум на 2 МГц.*/
	/*		7					|	6		|	5					|	4				|	3						|	2				1	|	0
		CONT_WAVE	|	res	|	RF_DR_LOW	|	PLL_LOCK	|	RF_DR_HIGH	|		RF_PWR	|	res	*/
	RF_SETUP		= 0x06,	//RW настройка радиоканала: скорость, мощность, коэффициент усиления
	//-----------------------------------------------------------------------------------------------------------------------------------
	STATUS			= 0x07,	//RW Регистр статуса (регистры прерывания, состояние буферов приема/передачи и тд
	/*
	//	7		|		6		|		5		|		4			|		3		2		1	|		0
	//	res	|	RX_DR	|	TX_DS	|	MAX_RT	|		RX_P_NO		|	TX_FULL
	RX_DR - Прерывание по получению пакета. Бит устанавливается в единицу, когда усройство в режиме приняло
	адресованый ему пакет с совпавшей контрольной суммой. Бит сбрасывается путём записи в него значения 1.
	Принятый пакет доступен в очереди приёмника, и может быть прочитан командой R_RX_PAYLOAD,
	либо удалён командой FLUSH_RX.

	TX_DS - Прерывание по успешной отправке пакета. Бит устанавливается в единицу, когда устройство
	в режиме передатчика успешно отправило пакет и, если включено автоподтверждение,
	приняло подтверждение получения. После успешной отправки пакет удаляется из очереди передатчика.
	Бит сбрасывается путём записи в него значения 1.

	MAX_RT - Прерывание по превышению числа попыток повторной отправки.
	Бит устанавливается в единицу, когда устройство в режиме передатчика предприняло заданное
	в регистре SETUP_RETR количество попыток отправки, но так и не получило подтверждение от удалённого устройства.
	Передаваемый пакет остаётся в очереди передатчика. Для удаления его можно воспользоваться командой FLUSH_TX.
	Дальнейшая коммуникация невозможна, пока этот бит установлен. Бит сбрасывается путём записи в него значения 1.

	Пока любой из бит RX_DR, TX_DS, MAX_RT установлен в единицу и соответствующее ему прерывание не запрещено
	(т.е. соответствующие из бит MASK_RX_DR, MASK_TX_DS, MASK_MAX_RT в регистре CONFIG равны нулю),
	то на выходе IRQ чипа устанавливается низкий уровень. Для сброса значений этих бит,
	необходимо записать регистр STATUS с этими битами, установленными в 1.

	RX_P_NO - три бита кодирующие номер канала, пакет по которому доступен в начале очереди приёмника для чтения.
	Значения 000 - 101 кодируют каналы с 0 по 5, соответственно, значение 111 указывает, что очередь приёмника пуста.

	TX_FULL - значение 1 этого бита показывает, что в очереди передатчика нет свободных слотов.
	*/
	//-----------------------------------------------------------------------------------------------------------------------------------
	/*	7	6	5	4	|	3	2	1	0
	PLOS_CNT	|	ARC_CNT		*/
	OBSERVE_TX	= 0x08,	//RW
	//-----------------------------------------------------------------------------------------------------------------------------------
	RPD					= 0x09,	//RW
	//-----------------------------------------------------------------------------------------------------------------------------------
/*8-битные регистры, задающие размер данных, принимаемых по каналам, соответственно 0-5,
если не включена поддержка произвольной длины пакетов в регистрах DYNPD и FEATURE.
Значение 0 указывает что канал не используется. Допустимы значения длины от 1 до 32.*/
	RX_ADDR_P0	= 0x0A,	//RW 3-5 байт (начиная с младшего байта). Адрес канала 0 приёмника
	RX_ADDR_P1	= 0x0B,	//RW 3-5 байт (начиная с младшего байта). Адрес канала 1 приёмника
	RX_ADDR_P2	= 0x0C,	//RW 3-5 байт (начиная с младшего байта). Адрес канала 2 приёмника
	RX_ADDR_P3	= 0x0D,	//RW 3-5 байт (начиная с младшего байта). Адрес канала 3 приёмника
	RX_ADDR_P4	= 0x0E,	//RW 3-5 байт (начиная с младшего байта). Адрес канала 4 приёмника
	RX_ADDR_P5	= 0x0F,	//RW 3-5 байт (начиная с младшего байта). Адрес канала 5 приёмника
	TX_ADDR		= 0x10,
	//-----------------------------------------------------------------------------------------------------------------------------------
	RX_PW_P0		= 0x11,	//RW допустимое число байт в канале 0: от 1 до 32. 0 - канал не используется.*/
	RX_PW_P1		= 0x12,	//RW допустимое число байт в канале 0: от 1 до 32. 0 - канал не используется.*/
	RX_PW_P2		= 0x13,	//RW допустимое число байт в канале 0: от 1 до 32. 0 - канал не используется.*/
	RX_PW_P3		= 0x14,	//RW допустимое число байт в канале 0: от 1 до 32. 0 - канал не используется.*/
	RX_PW_P4		= 0x15,	//RW допустимое число байт в канале 0: от 1 до 32. 0 - канал не используется.*/
	RX_PW_P5		= 0x16,	//RW допустимое число байт в канале 0: от 1 до 32. 0 - канал не используется.*/
	//-----------------------------------------------------------------------------------------------------------------------------------
	FIFO_STATUS = 0x17,	//RO Состояние очередей FIFO приёмника и передатчика
	//-----------------------------------------------------------------------------------------------------------------------------------
	/*	7	|		6	|		5			|		4			|		3			|		2			|		1			|		0
		res	|	res	|	DPL_P5	|	DPL_P4	|	DPL_P3	|	DPL_P2	|	DPL_P1	|	DPL_P0	*/
	DYNPD			= 0x1C,	//RW Выбор каналов приёмника для которых используется произвольная длина пакетов
											//DPL_Px разрешает приём пакетов произвольной длины по соответствующему каналу.
											//При этом такая опция должна быть включена установленным битом EN_DPL в регистре FEATURE,
											//а также включено автоподтверждение установкой соответствующего бита ENAA_Px в регистре EN_AA
											//В случае если бит принимает значение 0, то размер данных в принимаемых пакетах должен быть равен
											//значению соответствующего регистра RX_PW_Px
	//-----------------------------------------------------------------------------------------------------------------------------------
	FEATURE			= 0x1D		//RW Регистр опций
/*EN_DPL включает поддержку приёма и передачи пакетов с размером поля данных произвольной длины.
В этом случае приём пакетов произвольной длины по каналам должен быть разрешён в регистре DYNPD и включено автоподтверждение в регистре EN_AA.
Если опция отключена, то размер принимаемых данных задаётся значением регистров RX_PW_P0 - RX_PW_P5.
При передаче также передаётся поле, указывающее длительность пакета.
Длина передаваемого пакета определяется размером записанных командой W_TX_PAYLOAD данных и она должна соответствовать настройкам принимающей стороны.
EN_ACK_PAY включает поддержку передачи данных с пакетами подтверждения.
После включения этой опции, командой W_ACK_PAYLOAD в очередь передатчика могут быть помещены данные,
которые будут отправлены вместе с пакетом подтверждения приёма.
EN_DYN_ACK разрешает передавать пакеты, не требующие подтверждения приёма.
После включения этой опции, командой W_TX_PAYLOAD_NOACK в очередь передатчика могут быть помещены данные,
на которые удалённой стороне не требуется присылать подтверждение о приёме.*/
};


//----	ENAA setup		-------------------------------------------------------------------------------------
/*enum ENAA_Bits {
	ENAA_P5	= 0x20,	//RW Включает автоподтверждение данных по каналу 5; 1 - ON; 0 - OFF
	ENAA_P4	= 0x10, //RW Включает автоподтверждение данных по каналу 4; 1 - ON; 0 - OFF
	ENAA_P3	= 0x08, //RW Включает автоподтверждение данных по каналу 3; 1 - ON; 0 - OFF
	ENAA_P2	= 0x04, //RW Включает автоподтверждение данных по каналу 2; 1 - ON; 0 - OFF
	ENAA_P1	= 0x02, //RW Включает автоподтверждение данных по каналу 1; 1 - ON; 0 - OFF
	ENAA_P0	= 0x01 	//RW Включает автоподтверждение данных по каналу 0; 1 - ON; 0 - OFF
};
enum EN_RXADDR_Bits {
	EN_RXADDR_P5 = 0x20,	//RW включает канал 5 приёмника; 1 - ON | 0 - OFF
	EN_RXADDR_P4 = 0x10,	//RW включает канал 4 приёмника; 1 - ON | 0 - OFF
	EN_RXADDR_P3 = 0x08,	//RW включает канал 3 приёмника; 1 - ON | 0 - OFF
	EN_RXADDR_P2 = 0x04,	//RW включает канал 2 приёмника; 1 - ON | 0 - OFF
	EN_RXADDR_P1 = 0x02,	//RW включает канал 1 приёмника; 1 - ON | 0 - OFF
	EN_RXADDR_P0 = 0x01		//RW включает канал 0 приёмника; 1 - ON | 0 - OFF
};*/

//----	STATUS decode		-------------------------------------------------------------------------------------
enum STATUS_Bits {
	TX_FULL	= 0x01	//RO
};
//--------------------------------------------------------------------------------------------------------------------------
/*	7	|				6				|				5				|				4				|	3		|	2		|			1					|				0
	res	|	TX_FIFO_REUSE	|	TX_FIFO_FULL	|	TX_FIFO_EMPTY	|	res	|	res	|	RX_FIFO_FULL	|	RX_FIFO_EMPTY	*/
/*TX_REUSE Признак готовности последнего пакета для повтрной отправки. Устанавливается командой REUSE_TX_PL.
TX_FULL Флаг переполнения FIFO очереди передатчика: 0 - есть свободное место в очереди; 1 - очередь переполнена.
TX_EMPTY Флаг освобождения FIFO очереди передатчика: 0 - в очереди есть данные; 1 - очередь пуста.
RX_FULL Флаг переполнения FIFO очереди приёмника: 0 - есть свободное место в очереди; 1 - очередь переполнена.
RX_EMPTY - Флаг освобождения FIFO очереди приёмника: 0 - в очереди есть данные; 1 - очередь пуста.*/
enum FIFO_STATUS_Bits {
	TX_FIFO_REUSE	= 0x40, //RO индикация повторного ипользования последнего пакета для повторной отправки.
	TX_FIFO_FULL	= 0x20, //RO Флаг переполнения FIFO очереди передатчика. 1 - full, 0 - empty
	TX_FIFO_EMPTY	= 0x10, //RO Флаг освобождения FIFO очереди передатчика. 1 - empty, 0 - data in buffer
	RX_FIFO_FULL	= 0x02, //RO Флаг переполнения FIFO очереди приёмника. 1 - full, 0 - empty
	RX_FIFO_EMPTY	= 0x01 	//RO Флаг освобождения FIFO очереди приёмника. 1 - empty, 0 - data in buffer
};

/*enum DYNPD_Bits {
	DPL_P5 = 0x20,	//RW включает  5 приёмника; 1 - ON | 0 - OFF
	DPL_P4 = 0x10,	//RW включает  4 приёмника; 1 - ON | 0 - OFF
	DPL_P3 = 0x08,	//RW включает  3 приёмника; 1 - ON | 0 - OFF
	DPL_P2 = 0x04,	//RW включает  2 приёмника; 1 - ON | 0 - OFF
	DPL_P1 = 0x02,	//RW включает  1 приёмника; 1 - ON | 0 - OFF
	DPL_P0 = 0x01		//RW включает  0 приёмника; 1 - ON | 0 - OFF
};*/

/*	7	|		6	|		5	|	4		|	3		|		2			|			1				|			0
	res	|	res	|	res	|	res	|	res	|	EN_DPL	|	EN_ACK_PAY	|	EN_DYN_ACK	*/


