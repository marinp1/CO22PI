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
	if (millis() - sdcard.last_mosi < 1000)
	{
		Serial.println("Detected write");
		delay(1000);

		static SdFat *sd;
		while (!sd)
		{
			sd = sdcard.initialise();
			delay(100);
		}

		sdcard.getCsvFile(sd);

		/*
		char *fname;
		while (!fname)
		{
			fname = sdcard.getCsvFile(sd);
			delay(200);
		}

		Serial.println(fname);
		*/

		/*
		char *line;
		while (!line)
		{
			line = sdcard.getFirstLine(sd, fname);
			delay(200);
		}

		Serial.println(line);
		*/

		sd->end();
		sd = 0;

		delay(50);
		sdcard.giveControl();

		delay(1000);
	}
	else
	{
		delay(100);
	}
}

void blink()
{
	LED_ON;
	delay(50);
	LED_OFF;
	delay(50);
}
