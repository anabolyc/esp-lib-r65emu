#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#define PS2_RELASE 0xf0

#define LONG_PRESS_MS 1000

#define KPD_LEFT 0x14 			// P1 LEFT
#define KPD_MIDDLE 0x59     	// P1 FIRE
#define KPD_RIGHT 0x11      	// P1 RIGHT

#define KPD_LONG_LEFT 0x16  	// P1 START
#define KPD_LONG_MIDDLE 0x26	// P1 COIN
#define KPD_LONG_RIGHT 0x29 	// P1 PAUSE

#include <Ticker.h>

class Keypad
{
public:
	Keypad() {}
		
	/**
	 * Starts the keyboard "service" by registering the external interrupt.
	 * setting the pin modes correctly and driving those needed to high.
	 * The propably best place to call this method is in the setup routine.
	 */
	void begin(uint8_t pin_l, uint8_t pin_m, uint8_t pin_r);

	/**
	 * Returns true if there is a key event to be read, false if not.
	 */
	bool available();

	/**
	 * Returns the scancode last received from the keypad.
	 * If there is no char available, 0 is returned.
	 */
	unsigned peek();

	unsigned read2();

private:
	Ticker *tick = new Ticker(); 
};

#define is_down(scan) ((scan) < 0x100)

#endif
