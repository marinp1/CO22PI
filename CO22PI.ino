#include <SD.h>
#include <SPI.h>
#include <list>
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
	Verbose,
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
	case LogLevel::Verbose:
		return "DEBUG";
		break;
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

/* Return name of largest CSV file. */
auto getCsvFile()
{
	// All found CSV files
	std::list<String> file_list = {};

	File root = SD.open("/");
	if (!root)
	{
		log(LogLevel::Error, "Failed to open root directory!");
		return file_list;
	}

	uint32_t largest_file_size = 0;

	while (true)
	{
		File entry = root.openNextFile();
		if (!entry)
		{
			break;
		}
		log(LogLevel::Debug, "openNext loop increment");
		String name = String(entry.name());
		if (entry.isDirectory())
		{
			log(LogLevel::Debug, "Detected folder " + name);
		}
		else if (name.endsWith(".CSV") || name.endsWith(".csv"))
		{
			log(LogLevel::Debug, "Detected CSV file " + name);
			uint32_t file_size = entry.size();
			if (file_size > largest_file_size)
			{
				largest_file_size = file_size;
				file_list.push_front(name);
				log(LogLevel::Debug, "Push front (" + String(file_size) + ")");
			}
			else
			{
				file_list.push_back(name);
				log(LogLevel::Debug, "Push back (" + String(file_size) + ")");
			}
		}
		else
		{
			log(LogLevel::Debug, "Detected non-csv file " + name);
		}
		entry.close();
	}

	root.close();
	return file_list;
}

/* Return last line of given file. */
String readLastLineOfFile(String fname)
{
	File csvFile = SD.open(fname);
	if (!csvFile)
	{
		log(LogLevel::Error, "Failed to open file");
		return "";
	}

	String line = "";
	char buffer[64];
	int index = 0;

	while (csvFile.available() > 0)
	{
		// read character and store in buffer
		buffer[index] = csvFile.read();
		// if newline or carriagereturn encountered
		if (buffer[index] == '\n' || buffer[index] == '\r')
		{
			// replace newline or carriagereturn by string terminator character so we have a proper C-style string
			buffer[index] = '\0';
			// reset index so next line will fill buffer from beginning
			index = 0;
			// if there is actual text
			if (strlen(buffer) != 0)
			{
				line = String(buffer);
				log(LogLevel::Verbose, "Line: " + line);
			}
		}
		else
		{
			index++;
		}
	}

	csvFile.close();
	line.trim();
	return line;
}

/* Delete ALL files on SD card */
void deleteAllFiles(std::list<String> filesToDelete)
{
	std::list<String>::iterator it;
	for (it = filesToDelete.begin(); it != filesToDelete.end(); ++it)
	{
		String name = it->c_str();
		if (SD.remove(name))
		{
			log(LogLevel::Info, "Deleted " + name);
		}
		else
		{
			log(LogLevel::Error, "Failed to delete " + name);
		}
	}
	log(LogLevel::Debug, "Files deleted successfully");
}

void loop(void)
{
	if (should_read && (millis() - last_mosi > 2000))
	{
		should_read = false; // Mark read as one even on failure
		reading = true;

		log(LogLevel::Info, "Starting read procedure");

		if (SD.begin(SD_CS_PIN, SPI_QUARTER_SPEED))
		{

			auto file_list = getCsvFile();
			if (file_list.size() == 0)
			{

				log(LogLevel::Error, "Failed to find suitable CSV file");
			}
			else
			{
				log(LogLevel::Info, "Found file with name: " + file_list.front());
				String line = readLastLineOfFile(file_list.front());
				if (line.length() == 0)
				{

					log(LogLevel::Error, "Failed to read file");
				}
				else
				{
					Serial.println(line);
				}
				deleteAllFiles(file_list);
			}
		}
		else
		{
			log(LogLevel::Error, "Failed to start SD card");
		}

		SD.end();
		GIVE_SPI_CONTROL; // Set SPI as INPUT
		reading = false;
	}

	delay(1000); // Wait before trying to read again
}
