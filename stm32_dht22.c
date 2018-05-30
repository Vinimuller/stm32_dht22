#include "stm32_dht22.h"

struct s_dht22Data		dht22Data;

void DHT22_Init(void) {
	//Don't forget to enable DHT22_SDA_PORT clock at RCC->IOPENR register
	/*DHT_SDA pin config
	 * MODE Output
	 * Max Speed
	 * Set pin high
	*/
	DHT22_SDA_PORT->MODER	|= (uint32_t)(0x01 << DHT22_SDA_PIN * 2);
	DHT22_SDA_PORT->OSPEEDR	|= (uint32_t)(0x11 << DHT22_SDA_PIN * 2);
	DHT22_SDA_PORT->ODR		|= (uint32_t)(0x01 << DHT22_SDA_PIN);
}

uint32_t DHT22_GetReadings(void) {
	uint32_t wait;
	uint8_t i;

	// Generate start impulse for sensor
	DHT22_SDA_PORT->MODER	|= (uint32_t)(0x01 << DHT22_SDA_PIN * 2);	//Set SDA pin to output
	DHT22_SDA_PORT->ODR		&= ~(0x01 << DHT22_SDA_PIN); 				//Set SDA pin low and wait 2ms
	Delay_ms(2);
	DHT22_SDA_PORT->MODER	&= ~(0x11 << DHT22_SDA_PIN * 2);			//Set SDA pin as input
	DHT22_SDA_PORT->PUPDR	|= (0x01 << DHT22_SDA_PIN * 2);				//Internal pull up for SDA pin

	// Wait for AM2302 to start communicate
	wait = 0;
	while ((DHT22_SDA_PORT->IDR & (1 << DHT22_SDA_PIN)) && (wait++ < 15)) Delay_us(5);
	if (wait > 12) return DHT22_RCV_NO_RESPONSE;

	// Check ACK strobe from sensor
	while (!(DHT22_SDA_PORT->IDR & (1 << DHT22_SDA_PIN)) && (wait++ < 15)) Delay_us(5);
	if (wait > 12) return DHT22_RCV_BAD_ACK1;

	wait = 0;
	while ((DHT22_SDA_PORT->IDR & (1 << DHT22_SDA_PIN)) && (wait++ < 15)) Delay_us(5);
	if (wait > 12) return DHT22_RCV_BAD_ACK2;

	// ACK strobe received --> receive 40 bits
	i = 0;
	while (i < 40) {
		// Measure bit start impulse (T_low = 50us)
		wait = 0;
		while (!(DHT22_SDA_PORT->IDR & (1 << DHT22_SDA_PIN)) && (wait++ < 15)) Delay_us(5);
		if (wait > 15) {
			// invalid bit start impulse length
			dht22Data.bits[i] = 0xffff;
			while ((DHT22_SDA_PORT->IDR & (1 << DHT22_SDA_PIN)) && (wait++ < 20)) Delay_us(5);
		} else {
			// Measure bit impulse length (T_h0 = 25us, T_h1 = 70us)
			wait = 0;
			while ((DHT22_SDA_PORT->IDR & (1 << DHT22_SDA_PIN)) && (wait++ < 20)) Delay_us(5);
			dht22Data.bits[i] = (wait < 20) ? wait : 0xffff;
		}
		i++;
	}

	for (i = 0; i < 40; i++) if (dht22Data.bits[i] == 0xffff) return DHT22_RCV_RCV_TIMEOUT;		//Scan for invalid bits received

	if(DHT22_DecodeReadings()) return DHT22_PARITY_ERROR;

	return DHT22_RCV_OK;
}

uint16_t DHT22_DecodeReadings(void) {
	uint8_t parity;
	uint8_t  i = 0;

	//Parse received data in dht22Data.bits[] vector
	dht22Data.hMSB = 0;
	for (; i < 8; i++) {
		dht22Data.hMSB <<= 1;
		if (dht22Data.bits[i] > 6) dht22Data.hMSB |= 1;
	}
	dht22Data.hLSB = 0;
	for (; i < 16; i++) {
		dht22Data.hLSB <<= 1;
		if (dht22Data.bits[i] > 6) dht22Data.hLSB |= 1;
	}
	dht22Data.tMSB = 0;
	for (; i < 24; i++) {
		dht22Data.tMSB <<= 1;
		if (dht22Data.bits[i] > 6) dht22Data.tMSB |= 1;
	}
	dht22Data.tLSB = 0;
	for (; i < 32; i++) {
		dht22Data.tLSB <<= 1;
		if (dht22Data.bits[i] > 6) dht22Data.tLSB |= 1;
	}
	for (; i < 40; i++) {
		dht22Data.parity_rcv <<= 1;
		if (dht22Data.bits[i] > 6) dht22Data.parity_rcv |= 1;
	}

	parity  = dht22Data.hMSB + dht22Data.hLSB + dht22Data.tMSB + dht22Data.tLSB;

	return (dht22Data.parity_rcv ^ (uint8_t)(parity));
}

float DHT22_GetHumidity(void) {
	return ((dht22Data.hMSB << 8) + dht22Data.hLSB)/10.0;
}

float DHT22_GetTemperature(void) {
	return ((dht22Data.tMSB << 8) + dht22Data.tLSB)/10.0;
}

