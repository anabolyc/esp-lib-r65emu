#include "hardware.h"

#include <SPI.h>

#if defined(USE_SD)
#include <SD.h>
#elif defined(USE_SPIFFS)
#include <SPIFFS.h>
#elif defined(USE_FS)
#include <FS.h>
#endif

#include "memory.h"
#include "CPU.h"

#if defined(USE_SPIRAM)
#include <SpiRAM.h>
#include "spiram.h"
spiram sram(SPIRAM_SIZE);
#endif

Memory memory;

#if defined(USE_KBD)
#include "ps2drv.h"
PS2Driver ps2;
#endif

#if defined(USE_KEYPAD)
#include "keypad.h"
Keypad kpd; 
#endif

static CPU *_cpu;

bool hardware_reset() {
	bool success = true;

#if defined(USE_SPIRAM)
	extern SPIClass SPIRAM_DEV;
	SPIRAM_DEV.begin();
#if defined(SPIRAM_MODULE)
	SPIRAM_DEV.setModule(SPIRAM_MODULE);
#endif
#if defined(SPIRAM_CLKDIV)
	SPIRAM_DEV.setClockDivider(SPIRAM_CLKDIV);
#endif
	SPIRAM_DEV.setDataMode(SPI_MODE0);
#endif

#if defined(USE_SD)
#if defined(SD_SPI)
	success = SD.begin(SD_CS, 2, SD_SPI);
	pinMode(SPI_CS, OUTPUT);	// without this, the SPI-RAM isn't seen
#else
	success = SD.begin(SD_CS);
#endif

#elif defined(USE_SPIFFS)
	success = SPIFFS.begin(true);

#elif defined(USE_FS)
	success = SPIFFS.begin();
#endif

#if defined(TFT_BACKLIGHT)
	digitalWrite(TFT_BACKLIGHT, HIGH);
#endif

	_cpu->reset();
	return success;
}

void hardware_init(CPU &cpu) {
	_cpu = &cpu;
	memory.begin();

#if defined(USE_KBD)
	ps2.begin(KBD_DATA, KBD_IRQ);
#endif

#if defined(USE_KEYPAD)
	kpd.begin(PIN_BTN_L, PIN_BTN_M, PIN_BTN_R);
#endif

#if defined(TFT_BACKLIGHT)
	pinMode(TFT_BACKLIGHT, OUTPUT);
#endif

#if defined(USE_SD)
	pinMode(SD_CS, OUTPUT);
	digitalWrite(SD_CS, HIGH);
#endif

#if defined(USE_SPIRAM)
	pinMode(SPIRAM_CS, OUTPUT);
	digitalWrite(SPIRAM_CS, HIGH);
#endif
}

#if !defined(NO_CHECKPOINT)
void hardware_checkpoint(Stream &s) {
	unsigned ds = 0;
	for (unsigned i = 0; i < 0x10000; i += ds) {
		Memory::Device *dev = memory.get(i);
		dev->checkpoint(s);
		ds = dev->pages() * Memory::page_size;
	}
	_cpu->checkpoint(s);
}

void hardware_restore(Stream &s) {
	unsigned ds = 0;
	for (unsigned i = 0; i < 0x10000; i += ds) {
		Memory::Device *dev = memory.get(i);
		dev->restore(s);
		ds = dev->pages() * Memory::page_size;
	}
	_cpu->restore(s);
}
#endif
