#include "stm32l0xx.h"
#include "stm32_delay.h"

/*--- DHT22 for STM32 ---
 *
 * This is an adaptation of https://github.com/LonelyWolf/stm32/tree/master/am2302.
 * Reads DHT22 data based in a delay function of 5 microseconds that work with timers.
 *
 * Vinícius Müller Silveira - https://github.com/Vinimuller - 30/05/2018
 */

#define DHT22_SDA_PORT 	GPIOA
#define DHT22_SDA_PIN 	10

#define DHT22_RCV_OK			0
#define DHT22_RCV_NO_RESPONSE	1
#define DHT22_RCV_BAD_ACK1		2
#define DHT22_RCV_BAD_ACK2		3
#define DHT22_RCV_RCV_TIMEOUT 	4
#define DHT22_PARITY_ERROR		5

#define COUNTER_TIMEOUT		   1000
#define COUNTER_HIGH_THRESHOLD 60

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
