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

bool DEBUG_ACTIVE = false;

void setup()
{
	Serial.begin(9600);
	INIT_LED;
	LED_ON;

	Serial.println("");
	if (DEBUG_ACTIVE)
	{
		Serial.println("[INFO] Starting...");
	}

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
			if (DEBUG_ACTIVE) {
				Serial.println("MISO " + String(millis()));
		 }; },
		CHANGE);

	attachInterrupt(
		digitalPinToInterrupt(MOSI_PIN), []()
		{
			if (reading) return;
			last_mosi = millis();
			should_read = true;
			if (DEBUG_ACTIVE) {
				Serial.println("MOSI " + String(millis()));
		 }; },
		CHANGE);

	if (DEBUG_ACTIVE)
	{
		Serial.println("[INFO] Started!");
	}

	LED_OFF;
}

#define SPI_CLOCK SD_SCK_HZ(500000)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_HALF_SPEED)

String getCsvFile(SdFs *sd)
{

	File32 file, dir;
	char fname[64];

	uint16 latestdate = 0, latesttime = 0;
	uint16 moddate, modtime;

	String latest_fname = "";

	if (!dir.open("/"))
	{
		if (DEBUG_ACTIVE)
		{
			Serial.println("[ERR] Failed to open root directory!");
		}
	}
	else
	{
		while (file.openNext(&dir, O_READ))
		{
			file.clearWriteError();
			file.clearError();
			if (!file.isHidden() && file.isFile())
			{
				file.getName(fname, sizeof(fname));
				file.getModifyDateTime(&moddate, &modtime);
				if (String(fname).endsWith(".csv"))
				{
					if (moddate > latestdate && modtime > latesttime)
					{
						latest_fname = String(fname);
					}
				}
			}
			file.close();
		}
	}

	return latest_fname;
}

String readLastLineOfFile(SdFs *sd, String fname)
{
	char linetoread[64];
	char line[64] = "";

	FsFile file = sd->open(fname, O_RDONLY);
	file.rewind();

	int i = 2;
	while (i-- > 0)
	{
		int n = file.fgets(linetoread, 64);
		if (n <= 0)
		{
			if (DEBUG_ACTIVE)
			{
				Serial.println("[WARN] Read failed");
			}
			file.close();
			return "";
		}
		if (linetoread[n - 1] != '\n' && n == (sizeof(linetoread) - 1))
		{
			if (DEBUG_ACTIVE)
			{
				Serial.println("[WARN] Line too long");
			}
			file.close();
			return "";
		}
		strcpy(line, linetoread);
	}

	file.close();

	String lineData = String(line);
	lineData.trim();

	return lineData;
}

String deleteFile(SdFs *sd, String fname)
{
	if (sd->remove(fname))
	{
		if (DEBUG_ACTIVE)
		{
			Serial.println("[INFO] File deleted");
		}
	}
	else
	{
		if (DEBUG_ACTIVE)
		{
			Serial.println("[ERR] File deletion failed");
		}
	}
}

void loop(void)
{
	if (should_read)
	{
		if (millis() - last_mosi > 5000)
		{
			should_read = false;
			reading = true;

			if (DEBUG_ACTIVE)
			{
				Serial.println("[INFO] Trying to read " + String(millis()));
			}

			SdFs sd;
			if (sd.begin(SD_CONFIG))
			{
				String fname = getCsvFile(&sd);
				if (fname.length() == 0)
				{
					if (DEBUG_ACTIVE)
					{
						Serial.println("[ERR] Failed to find file");
					}
				}
				else
				{
					if (DEBUG_ACTIVE)
					{

						Serial.println("[INFO] Found: " + fname);
					}
					String line = readLastLineOfFile(&sd, fname);
					if (line.length() == 0)
					{
						if (DEBUG_ACTIVE)
						{

							Serial.println("[ERR] Failed to read file");
							deleteFile(&sd, fname);
						}
					}
					else
					{
						Serial.println(line);
						deleteFile(&sd, fname);
					}
				}
				sd.end();
			}
			else
			{
				if (DEBUG_ACTIVE)
				{
					Serial.println("[ERR] Failed to start SD card");
				}
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
