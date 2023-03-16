/**
 * Author Wojciech Domski <Wojciech.Domski@gmail.com>
 * www: www.Domski.pl
 *
 * work based on DORJI.COM sample code and
 * https://github.com/realspinner/SX1278_LoRa
 */

#ifndef __SX1278_H__
#define __SX1278_H__

#include <stdint.h>
#include <stdbool.h>

#include "SX1278_hw.h"

#define FXOSC 32000000
#define DOWNLINK_FREQ 150000000
#define UPLINK_FREQ 170000000
#define SX1278_MAX_PACKET	256
#define SX1278_DEFAULT_TIMEOUT		3000
#define LORA_SEND_TIMEOUT 2000 //2000
#define SX1278_POWER_20DBM  0xFF //20dbm
#define SX1278_POWER_17DBM  0xFC //17dbm
#define SX1278_POWER_14DBM  0xF9 //14dbm
#define SX1278_POWER_11DBM  0xF6 //11dbm
#define LORAWAN  0x34
#define OVERCURRENTPROTECT 0x0B //Default value
#define LNAGAIN 0x23 //Highest gain & Boost on
#define LNA_SET_BY_AGC 0x04
#define RX_TIMEOUT_LSB 0x08
#define PREAMBLE_LENGTH_MSB 0x00
#define PREAMBLE_LENGTH_LSB 8
#define HOPS_PERIOD 0x00
#define DIO0_1_2_3_CONFIG 0x41
#define FLAGS_VALUE 0xF7
//RFM98 Internal registers Address
/********************LoRa mode***************************/
#define LR_RegFifo                                  0x00
// Common settings
#define LR_RegOpMode                                0x01
#define LR_RegFrMsb                                 0x06
#define LR_RegFrMid                                 0x07
#define LR_RegFrLsb                                 0x08
// Tx settings
#define LR_RegPaConfig                              0x09
#define LR_RegPaRamp                                0x0A
#define LR_RegOcp                                   0x0B
// Rx settings
#define LR_RegLna                                   0x0C
// LoRa registers
#define LR_RegFifoAddrPtr                           0x0D
#define LR_RegFifoTxBaseAddr                        0x0E
#define LR_RegFifoRxBaseAddr                        0x0F
#define LR_RegFifoRxCurrentaddr                     0x10
#define LR_RegIrqFlagsMask                          0x11
#define LR_RegIrqFlags                              0x12
#define LR_RegRxNbBytes                             0x13
#define LR_RegRxHeaderCntValueMsb                   0x14
#define LR_RegRxHeaderCntValueLsb                   0x15
#define LR_RegRxPacketCntValueMsb                   0x16
#define LR_RegRxPacketCntValueLsb                   0x17
#define LR_RegModemStat                             0x18
#define LR_RegPktSnrValue                           0x19
#define LR_RegPktRssiValue                          0x1A
#define LR_RegRssiValue                             0x1B
#define LR_RegHopChannel                            0x1C
#define LR_RegModemConfig1                          0x1D
#define LR_RegModemConfig2                          0x1E
#define LR_RegSymbTimeoutLsb                        0x1F
#define LR_RegPreambleMsb                           0x20
#define LR_RegPreambleLsb                           0x21
#define LR_RegPayloadLength                         0x22
#define LR_RegMaxPayloadLength                      0x23
#define LR_RegHopPeriod                             0x24
#define LR_RegFifoRxByteAddr                        0x25
#define LR_RegModemConfig3                          0x26
#define LR_RegDetectOptimize						0x31
#define LR_RegDetectionThreshold					0x37
// I/O settings
#define LR_RegDioMapping1                          0x40
#define LR_RegDioMapping2                          0x41
// I/O settings
//#define REG_LR_DIOMAPPING1                          0x40
//#define REG_LR_DIOMAPPING2                          0x41
// Version
#define LR_RegVersion                              0x42
// Additional settings
#define LR_RegPllHop                               0x44
#define LR_RegTCXO                                 0x4B
#define LR_RegPaDac                                0x4D
#define LR_RegFormerTemp                           0x5B
#define LR_RegAgcRef                               0x61
#define LR_RegAgcThresh1                           0x62
#define LR_RegAgcThresh2                           0x63
#define LR_RegAgcThresh3                           0x64

// Additional settings
#define REG_LR_PLLHOP                               0x44
#define REG_LR_TCXO                                 0x4B
#define REG_LR_PADAC                                0x4D
#define REG_LR_FORMERTEMP                           0x5B
#define REG_LR_AGCREF                               0x61
#define REG_LR_AGCTHRESH1                           0x62
#define REG_LR_AGCTHRESH2                           0x63
#define REG_LR_AGCTHRESH3                           0x64

///// Direcciones de prueba
#define DIRECCION_0X80                              0x80
#define DIRECCION_0X81                              0x81
#define DIRECCION_0X82                              0x82
#define DIRECCION_0X83                              0x83
#define DIRECCION_0X84                              0x84
#define DIRECCION_0X85                              0x85
#define DIRECCION_0X86                              0x86
#define DIRECCION_0X87                              0x87

/********************FSK/ook mode***************************/
#define  RegFIFO                0x00
#define  RegOpMode              0x01
#define  RegBitRateMsb      	0x02
#define  RegBitRateLsb      	0x03
#define  RegFdevMsb             0x04
#define  RegFdevLsb             0x05
#define  RegFreqMsb             0x06
#define  RegFreqMid             0x07
#define  RegFreqLsb         	0x08
#define  RegPaConfig            0x09
#define  RegPaRamp              0x0a
#define  RegOcp                 0x0b
#define  RegLna                 0x0c
#define  RegRxConfig            0x0d
#define  RegRssiConfig      	0x0e
#define  RegRssiCollision 		0x0f
#define  RegRssiThresh      	0x10
#define  RegRssiValue           0x11
#define  RegRxBw                0x12
#define  RegAfcBw               0x13
#define  RegOokPeak             0x14
#define  RegOokFix              0x15
#define  RegOokAvg              0x16
#define  RegAfcFei              0x1a
#define  RegAfcMsb              0x1b
#define  RegAfcLsb              0x1c
#define  RegFeiMsb              0x1d
#define  RegFeiLsb              0x1e
#define  RegPreambleDetect  	0x1f
#define  RegRxTimeout1      	0x20
#define  RegRxTimeout2      	0x21
#define  RegRxTimeout3      	0x22
#define  RegRxDelay             0x23
#define  RegOsc                 0x24
#define  RegPreambleMsb     	0x25
#define  RegPreambleLsb     	0x26
#define  RegSyncConfig      	0x27
#define  RegSyncValue1      	0x28
#define  RegSyncValue2      	0x29
#define  RegSyncValue3      	0x2a
#define  RegSyncValue4      	0x2b
#define  RegSyncValue5      	0x2c
#define  RegSyncValue6      	0x2d
#define  RegSyncValue7      	0x2e
#define  RegSyncValue8      	0x2f
#define  RegPacketConfig1       0x30
#define  RegPacketConfig2       0x31
#define  RegPayloadLength       0x32
#define  RegNodeAdrs            0x33
#define  RegBroadcastAdrs       0x34
#define  RegFifoThresh      	0x35
#define  RegSeqConfig1      	0x36
#define  RegSeqConfig2      	0x37
#define  RegTimerResol      	0x38
#define  RegTimer1Coef      	0x39
#define  RegSyncWord			0x39
#define  RegTimer2Coef      	0x3a
#define  RegImageCal            0x3b
#define  RegTemp                0x3c
#define  RegLowBat              0x3d
#define  RegIrqFlags1           0x3e
#define  RegIrqFlags2           0x3f
#define  RegDioMapping1			0x40
#define  RegDioMapping2			0x41
#define  RegVersion				0x42
#define  RegPllHop				0x44
#define  RegPaDac				0x4d
#define  RegBitRateFrac			0x5d

/**********************************************************
 **Parameter table define
 **********************************************************/

#define RX_TIMEOUT_MASK            (0x1 << 7)       /*!< 0x00000020 */
#define RX_DONE_MASK               (0x1 << 6)
#define PAYLOAD_CRC_ERROR_MASK     (0x1 << 5)
#define VALID_HEADER_MASK          (0x1 << 4)
#define TX_DONE_MASK               (0x1 << 3)
#define CAD_DONE_MASK              (0x1 << 2)
#define FHSS_CHANGE_CHANNEL_MASK   (0x1 << 1)
#define CAD_DETECTED_MASK          (0x1 << 0)

typedef enum SPREAD_FACTOR {
	SF_6 = 6, SF_7, SF_8, SF_9, SF_10, SF_11, SF_12
} SPREAD_FACTOR_t;
static const uint8_t SX1278_SpreadFactor[7] = { 6, 7, 8, 9, 10, 11, 12 };

#define	SX1278_LORA_BW_7_8KHZ		0
#define	SX1278_LORA_BW_10_4KHZ		1
#define	SX1278_LORA_BW_15_6KHZ		2
#define	SX1278_LORA_BW_20_8KHZ		3
#define	SX1278_LORA_BW_31_2KHZ		4
#define	SX1278_LORA_BW_41_7KHZ		5
#define	SX1278_LORA_BW_62_5KHZ		6
#define	SX1278_LORA_BW_125KHZ		7
#define	SX1278_LORA_BW_250KHZ		8
#define	SX1278_LORA_BW_500KHZ		9

static const uint8_t SX1278_LoRaBandwidth[10] = { 0, //   7.8KHz,
		1, //  10.4KHz,
		2, //  15.6KHz,
		3, //  20.8KHz,
		4, //  31.2KHz,
		5, //  41.7KHz,
		6, //  62.5KHz,
		7, // 125.0KHz,
		8, // 250.0KHz,
		9  // 500.0KHz
		};

enum LORABW {
	LORABW_7_8KHZ,
	LORABW_10_4KHZ,
	LORABW_15_6KHZ,
	LORABW_20_8KHZ,
	LORABW_31_2KHZ,
	LORABW_41_7KHZ,
	LORABW_62_5KHZ,
	LORABW_125KHZ,
	LORABW_250KHZ,
	LORABW_500KHZ
};

typedef enum CODING_RATE {
	LORA_CR_4_5 = 1, LORA_CR_4_6, LORA_CR_4_7, LORA_CR_4_8
} CODING_RATE_t;

typedef enum HEADER_MODE {
	EXPLICIT, IMPLICIT
} HEADER_MODE_t;

//Coding rate
#define SX1278_LORA_CR_4_5    0
#define SX1278_LORA_CR_4_6    1
#define SX1278_LORA_CR_4_7    2
#define SX1278_LORA_CR_4_8    3

static const uint8_t SX1278_CodingRate[4] = { 0x01, 0x02, 0x03, 0x04 };

//CRC Enable
#define SX1278_LORA_CRC_EN              0
#define SX1278_LORA_CRC_DIS             1

static const uint8_t SX1278_CRC_Sum[2] = { 0x01, 0x00 };

typedef enum CRC_SUM {
	CRC_DISABLE, CRC_ENABLE
} CRC_SUM_t;

typedef enum OPERATING_MODE {
	SLEEP,
	STANDBY,
	FSTX, //Frequency synthesis TX
	TX,
	FSRX, //Frequency synthesis RX
	RX_CONTINUOUS,
	RX_SINGLE,
	CAD //Channel activity detection
} OPERATING_MODE_t;

typedef enum SX1278_STATUS {
	UNKNOW,
	TX_READY,
	RX_READY
} SX1278_Status_t;

#define LORA_MODE_ACTIVATION (0x00 | 8 << 4)
#define HIGH_FREQUENCY_MODE (0x00 | 0 << 3)
#define LOW_FREQUENCY_MODE (0x00 | 1 << 3)

#define DIO0_RX_DONE (0x00 | 0 << 6)
#define DIO0_TX_DONE (0x00 | 1 << 6)
#define DIO0_CAD_DONE (0x00 | 2 << 6)
#define DIO1_RX_TIMEOUT (0x00 | 0 << 4)
#define DIO1_FHSS_CHANGE_CHANNEL (0x00 | 1 << 4)
#define DIO1_CAD_DETECTED (0x00 | 2 << 4)
#define DIO2_FHSS_CHANGE_CHANNEL (0x00 | 0 << 2)
#define DIO3_CAD_DONE (0x00 | 0 << 0)
#define DIO3_VALID_HEADER (0x00 | 1 << 0)
#define DIO3_PAYLOAD_CRC_ERROR (0x00 | 2 << 0)

#define MASK_ENABLE 0
#define MASK_DISABLE 1

typedef struct {
	SX1278_hw_t *hw;

	uint64_t frequency;
	uint8_t power;
	SPREAD_FACTOR_t LoRa_SF;
	uint8_t LoRa_BW;
	CODING_RATE_t LoRa_CR;
	CRC_SUM_t LoRa_CRC_sum;
	uint8_t packetLength;
	uint8_t syncWord;
	uint8_t ocp;
	uint8_t lnaGain;
	uint8_t AgcAutoOn;
	uint8_t symbTimeoutLsb;
	uint8_t symbTimeoutMsb;
	uint8_t preambleLengthMsb;
	uint8_t preambleLengthLsb;
	uint8_t fhssValue;
	uint8_t dioConfig;
	uint8_t flagsMode;
	HEADER_MODE_t headerMode;
	bool saveParameters;

	OPERATING_MODE_t operatingMode;

	SX1278_Status_t status;

	uint8_t buffer[SX1278_MAX_PACKET];
	uint8_t readBytes;

	SPI_HandleTypeDef *spi;
} SX1278_t;

/**
 * \brief Read byte from LoRa module
 *
 * Reads data from LoRa module from given address.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  addr		Address from which data will be read
 *
 * \return              Read data
 */
uint8_t SX1278_SPIRead(SX1278_t *module, uint8_t addr, SPI_HandleTypeDef *spi);

/**
 * \brief Write byte to LoRa module
 *
 * Writes data to LoRa module under given address.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  addr		Address under which data will be written
 * \param[in]  cmd 		Data to write
 */
void SX1278_SPIWrite(SX1278_t *module, uint8_t addr, uint8_t cmd,
		SPI_HandleTypeDef *spi);

/**
 * \brief Read data from LoRa module
 *
 * Reads data from LoRa module from given address.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  addr		Address from which data will be read
 * \param[out] rxBuf    Pointer to store read data
 * \param[in]  length   Number of bytes to read
 */
void SX1278_SPIBurstRead(SX1278_t *module, uint8_t addr, uint8_t *rxBuf,
		uint8_t length, SPI_HandleTypeDef *spi);

/**
 * \brief Write data to LoRa module
 *
 * Writes data to LoRa module under given address.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  addr		Address under which data will be written
 * \param[in]  txBuf    Pointer to data
 * \param[in]  length   Number of bytes to write
 */
void SX1278_SPIBurstWrite(SX1278_t *module, uint8_t addr, uint8_t *txBuf,
		uint8_t length, SPI_HandleTypeDef *spi);
void SX1278_SPIBurstWrite2(SX1278_t *module);

/**
 * \brief Configure LoRa module
 *
 * Configure LoRa module according to parameters stored in
 * module structure.
 *
 * \param[in]  module	Pointer to LoRa structure
 */
void configLoraMode(SX1278_t *module, SPI_HandleTypeDef *spi);

/**
 * \brief Entry LoRa mode
 *
 * Module supports different operation mode.
 * To use LoRa operation mode one has to enter this
 * particular mode to transmit and receive data
 * using LoRa.
 *
 * \param[in]  module	Pointer to LoRa structure
 */

/**
 * \brief Entry reception mode
 *
 * Entry reception mode
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  length   Length of message to be received
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entering reception mode
 *             0 if timeout was exceeded
 */
int SX1278_LoRaEntryRx(SX1278_t *module, uint8_t length, uint32_t timeout,
		SPI_HandleTypeDef *spi);

/**
 * \brief Read data
 *
 * Read data and store it in module's RX buffer
 *
 * \param[in]  module	Pointer to LoRa structure
 *
 * \return     returns number of read bytes
 */
uint8_t SX1278_LoRaRxPacket(SX1278_t *module, SPI_HandleTypeDef *spi);

/**
 * \brief Entry transmitter mode
 *
 * Entry transmitter mode
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  length   Length of message to be sent
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entering reception mode
 *             0 if timeout was exceeded
 */
int configTxRegister(SX1278_t *module, SPI_HandleTypeDef *spi);

/**
 * \brief Send data
 *
 * Transmit data
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  txBuf    Data buffer with data to be sent
 * \param[in]  length   Length of message to be sent
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entering reception mode
 *             0 if timeout was exceeded
 */
SX1278_Status_t SX1278_LoRaTxPacket(SX1278_t *module, SPI_HandleTypeDef *spi);

/**
 * \brief Initialize LoRa module
 *
 * Initialize LoRa module and initialize LoRa structure.
 *
 * \param[in]  module	    Pointer to LoRa structure
 * \param[in]  frequency    Frequency in [Hz]
 * \param[in]  power        Power level, accepts SX1278_POWER_*
 * \param[in]  LoRa_SF      LoRa spread rate, accepts SX1278_LORA_SF_*
 * \param[in]  LoRa_BW      LoRa bandwidth, accepts SX1278_LORA_BW_*
 * \param[in]  LoRa_CR      LoRa coding rate, accepts SX1278_LORA_CR_*
 * \param[in]  LoRa_CRC_sum Hardware CRC check, SX1278_LORA_CRC_EN or
 *                          SX1278_LORA_CRC_DIS
 * \param[in]  packetLength Package length, no more than 256 bytes
 */
void SX1278_init(SX1278_t *module, uint64_t frequency, uint8_t power,
		uint8_t LoRa_SF, uint8_t LoRa_BW, uint8_t LoRa_CR, uint8_t LoRa_CRC_sum,
		uint8_t packetLength, SPI_HandleTypeDef *spi);

/**
 * \brief Entry transmitter mode and send data
 *
 * Entry transmitter mode and send data.
 * Combination of SX1278_LoRaEntryTx() and SX1278_LoRaTxPacket().
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  txBuf    Data buffer with data to be sent
 * \param[in]  length   Length of message to be sent
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entered TX mode and sent data
 *             0 if timeout was exceeded
 */
int SX1278_transmit(SX1278_t *module, uint8_t *txBuf, uint8_t length,
		uint32_t timeout, SPI_HandleTypeDef *spi);

/**
 * \brief Entry reception mode
 *
 * Same as SX1278_LoRaEntryRx()
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[in]  length   Length of message to be received
 * \param[in]  timeout  Timeout in [ms]
 *
 * \return     1 if entering reception mode
 *             0 if timeout was exceeded
 */
int SX1278_receive(SX1278_t *module, uint8_t length, uint32_t timeout,
		SPI_HandleTypeDef *spi);

/**
 * \brief Returns number of received data
 *
 * Returns the number of received data which are
 * held in internal buffer.
 * Same as SX1278_LoRaRxPacket().
 *
 * \param[in]  module	Pointer to LoRa structure
 *
 * \return     returns number of read bytes
 */
uint8_t SX1278_available(SX1278_t *module, SPI_HandleTypeDef *spi);

/**
 * \brief Read received data to buffer
 *
 * Reads data from internal buffer to external
 * buffer. Reads exactly number of bytes which are stored in
 * internal buffer.
 *
 * \param[in]  module	Pointer to LoRa structure
 * \param[out] rxBuf    External buffer to store data.
 *                      External buffer is terminated with '\0'
 *                      character
 * \param[in]  length   Length of message to be received
 *
 * \return     returns number of read bytes
 */
uint8_t SX1278_read(SX1278_t *module, uint8_t *rxBuf, uint8_t length);

/**
 * \brief Returns RSSI (LoRa)
 *
 * Returns RSSI in LoRa mode.
 *
 * \param[in]  module	Pointer to LoRa structure
 *
 * \return     RSSI value
 */
uint8_t SX1278_RSSI_LoRa(SX1278_t *module, SPI_HandleTypeDef *spi);

/**
 * \brief Returns RSSI
 *
 * Returns RSSI (general mode).
 *
 * \param[in]  module	Pointer to LoRa structure
 *
 * \return     RSSI value
 */
uint8_t SX1278_RSSI(SX1278_t *module, SPI_HandleTypeDef *spi);

////////////////////////////////////////////////////////////////////////////////////
uint8_t readRegister(SPI_HandleTypeDef *spi, uint8_t address);
uint8_t writeRegister(SPI_HandleTypeDef *spi, uint8_t address, uint8_t *cmd,
		uint8_t lenght);
void setRFFrequency(SX1278_t *module);
void setOutputPower(SX1278_t *module);
void setLORAWAN(SX1278_t *module);
void setOvercurrentProtect(SX1278_t *module);
void setLNAGain(SX1278_t *module);
void setPreambleParameters(SX1278_t *module);
void setReModemConfig(SX1278_t *module);
void setDetectionParameters(SX1278_t *module);
void standbyMode(SX1278_t *module);
void sleepMode(SX1278_t *module);
void setMode(SX1278_t *module, SX1278_Status_t mode);
void setLoraLowFreqMode(SX1278_t *module);
void clearIrqFlags(SX1278_t *module);
SX1278_Status_t readMode(SX1278_t *module);
void updateLoraLowFreq(SX1278_t *module, SX1278_Status_t mode);
#endif
