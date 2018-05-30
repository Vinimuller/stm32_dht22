#include "stm32l0xx.h"
#include "stm32_delay.h"

#define DHT22_SDA_PORT 	GPIOA
#define DHT22_SDA_PIN 	10

#define DHT22_RCV_OK			0
#define DHT22_RCV_NO_RESPONSE	1
#define DHT22_RCV_BAD_ACK1		2
#define DHT22_RCV_BAD_ACK2		3
#define DHT22_RCV_RCV_TIMEOUT 	4
#define DHT22_PARITY_ERROR		5

struct s_dht22Data {
	uint8_t  hMSB;
	uint8_t  hLSB;
	uint8_t  tMSB;
	uint8_t  tLSB;
	uint8_t  parity_rcv;
	uint16_t bits[40];
};

void DHT22_init(void);
uint32_t DHT22_GetReadings(void);
uint16_t DHT22_DecodeReadings(void);
float DHT22_GetHumidity(void);
float DHT22_GetTemperature(void);
