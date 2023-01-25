#include <Arduino.h>
#include "keypad.h"

#if !defined(KPD_BUFFER)
#define KPD_BUFFER 16
#endif

static volatile uint8_t buffer[KPD_BUFFER];
static volatile uint8_t head, tail;
static uint8_t PinL, PinM, PinR;
static uint32_t pin_l_down = 0, pin_m_down = 0, pin_r_down = 0;
static uint32_t pin_l_up = 1, pin_m_up = 1, pin_r_up = 1;
static uint8_t pin_l_lp = 0, pin_m_lp = 0, pin_r_lp = 0;


bool Keypad::available() {
	if (head == tail)
		return false;

	uint8_t i = tail + 1;
	if (i == KPD_BUFFER) 
		i = 0;
	if (buffer[i] == PS2_RELASE)
		return i != head;
	return true;
}

unsigned Keypad::read2() {
	if (head == tail)
		return 0;

	uint8_t i = tail + 1;
	if (i == KPD_BUFFER) 
		i = 0;
	tail = i;
	if (buffer[i] != PS2_RELASE)
		return buffer[i];
	return 0xf000 | read2();
}

unsigned Keypad::peek() {
	if (head == tail)
		return 0;

	uint8_t i = tail + 1;
	if (i == KPD_BUFFER) 
		i = 0;
	if (buffer[i] == PS2_RELASE) {
		if (++i == KPD_BUFFER) 
			i = 0;
		return 0xf000 | buffer[i];
	}
	return buffer[i];
}

inline void send(uint8_t key) {
	uint8_t i = head + 1;		
	if (i == KPD_BUFFER) 
		i = 0;
	if (i != tail) {
		buffer[i] = key;
		head = i;
	}
}

#if defined(ESP32) || defined(ESP8266)
IRAM_ATTR
#endif
void pin_l_chg(void)
{
	if (digitalRead(PinL) == LOW) {
		send(PS2_RELASE);
		pin_l_up = millis();
	} else {
		pin_l_down = millis();
	}
	send(KPD_LEFT);
}

#if defined(ESP32) || defined(ESP8266)
IRAM_ATTR
#endif
void pin_r_chg(void)
{
	if (digitalRead(PinR) == LOW){
		send(PS2_RELASE);
		pin_r_up = millis();
	} else {
		pin_r_down = millis();
	}
	send(KPD_RIGHT);
}

#if defined(ESP32) || defined(ESP8266)
IRAM_ATTR
#endif
void pin_m_chg(void)
{
	if (digitalRead(PinM) == LOW){
		send(PS2_RELASE);
		pin_m_up = millis();
	} else {
		pin_m_down = millis();
	}
	send(KPD_MIDDLE);
}

void Keypad::begin(uint8_t pin_l, uint8_t pin_m, uint8_t pin_r)
{
	PinL = pin_l;
	PinM = pin_m;
	PinR = pin_r;

	pinMode(PinL, INPUT);
	pinMode(PinM, INPUT);
	pinMode(PinR, INPUT);

	tick->attach_ms(100, [](){
		uint32_t now = millis();
		
		if (pin_l_down < pin_l_up) {
			pin_l_lp = 0;
		}
		else if (now > pin_l_down + LONG_PRESS_MS) {
			if (pin_l_lp == 0) {
				send(KPD_LONG_LEFT);
				pin_l_lp = 1;
			} else if(pin_m_lp == 1) {
				send(PS2_RELASE);
				send(KPD_LONG_LEFT);
				pin_l_lp = 2;
			}
		}

		if (pin_m_down < pin_m_up) {
			pin_m_lp = 0;
		}
		else if (now > pin_m_down + LONG_PRESS_MS) {
			if (pin_m_lp == 0) {
				send(KPD_LONG_MIDDLE);
				pin_m_lp = 1;
			} else if(pin_m_lp == 1) {
				send(PS2_RELASE);
				send(KPD_LONG_MIDDLE);
				pin_m_lp = 2;
			}
		}

		if (pin_r_down < pin_r_up) {
			pin_r_lp = 0;
		}
		else if (now > pin_r_down + LONG_PRESS_MS) {
			if (pin_r_lp == 0) {
				send(KPD_LONG_RIGHT);
				pin_r_lp = 1;
			} else if(pin_r_lp == 1) {
				send(PS2_RELASE);
				send(KPD_LONG_RIGHT);
				pin_r_lp = 2;
			}
		}
	});

	attachInterrupt(PinL, pin_l_chg, CHANGE);
	attachInterrupt(PinM, pin_m_chg, CHANGE);
	attachInterrupt(PinR, pin_r_chg, CHANGE);

	head = tail = 0;
}
