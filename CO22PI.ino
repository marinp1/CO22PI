#include <SdFat.h>
#include <SPI.h>
#include <list>
// Include pin related data
#include "pins.h"

// Keep track of timestamps when pins have changed value
volatile int last_miso = 0;
volatile int last_mosi = 0;
// Keeps track if MOSI_PIN has received new value after read
volatile bool should_read = false;
// Active when program reading data from SD card
volatile bool reading = false;

enum LogLevel
{
	Debug,
	Info,
	Error,
	None,
};

// Logs to show
const LogLevel APP_LOG_LEVEL = LogLevel::None;

String logLevelToText(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Debug:
		return "DEBUG";
		break;
	case LogLevel::Info:
		return "INFO";
		break;
	case LogLevel::Error:
		return "ERROR";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

void log(LogLevel level, String line)
{
	if (APP_LOG_LEVEL == LogLevel::None)
	{
		return;
	}
	if (level >= APP_LOG_LEVEL)
	{
		Serial.println("[" + logLevelToText(level) + "] " + String(millis()) + " " + line);
	}
}

void attachInterrupts()
{
	attachInterrupt(
		digitalPinToInterrupt(MISO_PIN), []()
		{ 
			if (reading) return;
			last_miso = millis();
			log(LogLevel::Debug, "Detected MISO change"); },
		CHANGE);

	attachInterrupt(
		digitalPinToInterrupt(MOSI_PIN), []()
		{
			if (reading) return;
			last_mosi = millis();
			should_read = true;
			log(LogLevel::Debug, "Detected MOSI change"); },
		CHANGE);
}

void setup()
{
	log(LogLevel::Info, "Starting...");
	INIT_LED;
	LED_ON;

	Serial.begin(9600);
	Serial.println("");

	while (!Serial)
	{
		yield();
	}

	delay(1000);
	GIVE_SPI_CONTROL;	// Make sure that SD card is in INPUT mode
	attachInterrupts(); // Listen to MOSI and MISO changes

	LED_OFF;
	log(LogLevel::Info, "Started!");
}

struct CsvFileResponse
{
	String latest_fname;		 // Filename to return
	std::list<String> file_list; // List of all files Found
};

/* Return name of last modified CSV file.  */
auto getCsvFile(SdFs *sd)
{
	sd->chvol();

	File32 file, dir;

	// Response data
	CsvFileResponse response;
	response.latest_fname = "";
	response.file_list = {};

	if (!dir.open("/"))
	{

		log(LogLevel::Error, "Failed to open root directory!");
		dir.close();
		return response;
	}

	// Variables to keep track
	char fname[64];
	uint16 latestdate = 0, latesttime = 0;
	uint16 moddate, modtime;

	while (file.openNext(&dir, O_READ))
	{
		file.clearWriteError();
		file.clearError();
		if (!file.isDir())
		{
			file.getName(fname, sizeof(fname));
			response.file_list.push_back(String(fname));
			if (!file.isHidden())
			{
				file.getModifyDateTime(&moddate, &modtime);
				if (String(fname).endsWith(".csv") && moddate > latestdate && modtime > latesttime)
				{

					response.latest_fname = String(fname);
				}
			}
		}
		file.close();
	}

	return response;
}

/* Return 2nd line of given file. */
String readLastLineOfFile(SdFs *sd, String fname)
{
	sd->chvol();

	// Open file in read only mode
	FsFile file = sd->open(fname, O_RDONLY);
	file.rewind();

	String line;
	char linetoread[64];

	int i = 2;
	while (i-- > 0)
	{
		int n = file.fgets(linetoread, 64); // Line length should always be <<< 64
		if (n <= 0)
		{
			log(LogLevel::Error, "Reading file failed (fgets failed)");
			file.close();
			return "";
		}
		if (linetoread[n - 1] != '\n' && n == (sizeof(linetoread) - 1))
		{
			log(LogLevel::Error, "Reading file failed (line too long)");
			file.close();
			return "";
		}
		line = String(linetoread);
	}

	file.close();
	line.trim();
	return line;
}

/* Delete ALL files on SD card */
void deleteAllFiles(SdFs *sd, std::list<String> filesToDelete)
{
	sd->chvol();

	std::list<String>::iterator it;
	for (it = filesToDelete.begin(); it != filesToDelete.end(); ++it)
	{
		String name = it->c_str();
		if (sd->remove(name))
		{
			log(LogLevel::Info, "Deleted " + name);
		}
		else
		{
			log(LogLevel::Error, "Failed to delete " + name);
		}
	}

	log(LogLevel::Debug, "Procedure deleteAllFiles complete");
}

void loop(void)
{
	if (should_read && (millis() - last_mosi > 5000))
	{
		should_read = false; // Mark read as one even on failure
		reading = true;

		log(LogLevel::Info, "Starting read procedure");

		SdFs sd;
		if (sd.begin(SD_CONFIG))
		{

			auto csvResponse = getCsvFile(&sd);
			if (csvResponse.latest_fname.length() == 0)
			{

				log(LogLevel::Error, "Failed to find suitable CSV file");
			}
			else
			{
				log(LogLevel::Info, "Found file with name: " + csvResponse.latest_fname);
				String line = readLastLineOfFile(&sd, csvResponse.latest_fname);
				if (line.length() == 0)
				{

					log(LogLevel::Error, "Failed to read file");
				}
				else
				{
					Serial.println(line);
				}
			}
			deleteAllFiles(&sd, csvResponse.file_list);
			sd.end();
		}
		else
		{
			log(LogLevel::Error, "Failed to start SD card");
		}

		GIVE_SPI_CONTROL; // Set SPI as INPUT
		reading = false;
	}

	delay(1000); // Wait before trying to read again
}
