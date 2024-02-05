#include <SdFat.h>
#include <SPI.h>
#include "pins.h"

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

volatile int last_miso = 0;
volatile int last_mosi = 0;

volatile bool should_read = false;
volatile bool reading = false;

void setup()
{
	Serial.begin(9600);
	INIT_LED;
	LED_ON;

	Serial.println("");
	Serial.println("[INFO] Starting...");

	while (!Serial)
	{
		yield();
	}

	delay(1000);

	pinMode(MISO_PIN, INPUT);
	pinMode(MOSI_PIN, INPUT);
	pinMode(SCLK_PIN, INPUT);
	pinMode(SD_CS_PIN, INPUT);

	attachInterrupt(
		digitalPinToInterrupt(MISO_PIN), []()
		{ 
			if (reading) return;
			last_miso = millis();
			Serial.println("MISO " + String(millis())); },
		CHANGE);

	attachInterrupt(
		digitalPinToInterrupt(MOSI_PIN), []()
		{
			if (reading) return;
			last_mosi = millis();
			should_read = true;
			Serial.println("MOSI " + String(millis())); },
		CHANGE);

	Serial.println("[INFO] Started!");

	LED_OFF;
}

#define SPI_CLOCK SD_SCK_HZ(500000)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_HALF_SPEED)

char *CSV_EXT = ".csv";
bool DEBUG_ACTIVE = false;

void loop(void)
{
	if (should_read)
	{
		if (millis() - last_mosi > 5000)
		{
			should_read = false;
			reading = true;

			Serial.println("[INFO] Trying to read " + String(millis()));

			pinMode(MISO_PIN, SPECIAL);
			pinMode(MOSI_PIN, SPECIAL);
			pinMode(SCLK_PIN, SPECIAL);
			pinMode(SD_CS_PIN, OUTPUT);

			SdFs sd;
			if (sd.begin(SD_CONFIG))
			{
				sd.ls();
				sd.end();
			}
			else
			{
				Serial.println("[ERR] Failed to start SD card");
			}

			pinMode(MISO_PIN, INPUT);
			pinMode(MOSI_PIN, INPUT);
			pinMode(SCLK_PIN, INPUT);
			pinMode(SD_CS_PIN, INPUT);

			reading = false;
			should_read = false;
		}
	}

	delay(1000);
}

void blink()
{
	LED_ON;
	delay(50);
	LED_OFF;
	delay(50);
}
