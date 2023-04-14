#include "sdcard.h"

#define INIT_LED            \
	{                       \
		pinMode(2, OUTPUT); \
	}
#define LED_ON                \
	{                         \
		digitalWrite(2, LOW); \
	}
#define LED_OFF                \
	{                          \
		digitalWrite(2, HIGH); \
	}
// ------------------------
void setup()
{
	Serial.begin(9600);
	INIT_LED;

	LED_ON;

	while (!Serial)
	{
		yield();
	}

	sdcard.attachInterrupts();
	sdcard.giveControl();
	delay(2000);
	LED_OFF;
}

void loop()
{

	SdFat sd;
	char fname[64], line[64];

	static bool should_read = false;

	if (should_read)
	{
		sdcard.takeControl();
		while (!sdcard.initialise(&sd))
		{
			delay(100);
		}

		// Serial.println("Trying to read data");

		if (sdcard.getCsvFile(&sd, fname) && sdcard.getFirstLine(&sd, fname, line))
		{
			Serial.print(line);
			delay(200);
			sdcard.deleteFile(&sd, fname);
			should_read = false;
			sd.end();
			sdcard.giveControl();
			delay(1000);
		}
		else
		{
			// Serial.println("Failed to find data");
			sd.end();
			sdcard.giveControl();
			delay(100);
		}
	}
	else if (millis() - sdcard.last_mosi < 500)
	{
		// Serial.println("Detected write");
		should_read = true;
		delay(1000);
	}
	else
	{
		yield();
	}
}

void blink()
{
	LED_ON;
	delay(50);
	LED_OFF;
	delay(50);
}
