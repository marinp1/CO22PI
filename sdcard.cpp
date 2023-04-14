#include "sdcard.h"
#include "pins.h"

#define SPI_CLOCK SD_SCK_MHZ(1)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)

bool SDCard::initialise(SdFat *sd)
{
    if (!sd->begin(SD_CS_PIN, SPI_CLOCK))
    {
        // sd->errorPrint(&Serial);
        // Serial.println("[WARN] Failed to init sd");
        return false;
    }
    return true;
};

static char csv_ext[5] = ".csv";

bool SDCard::getCsvFile(SdFat *sd, char *latest_fname)
{
    File32 file, dir;
    char fname[64];

    uint16 latestdate = 0, latesttime = 0;
    uint16 moddate, modtime;
    bool found = false;

    if (!dir.open("/"))
    {
        // Serial.println("[WARN] Failed to open root dir");
    }
    else
    {
        while (file.openNext(&dir, O_READ))
        {
            if (!file.isHidden())
            {
                if (!file.isDir())
                {
                    file.getName(fname, sizeof(fname));
                    file.getModifyDateTime(&moddate, &modtime);
                    if (moddate > latestdate && modtime > latesttime)
                    {
                        strcpy(latest_fname, fname);
                        found = true;
                    }
                }
            }
            file.close();
        }
    }

    return found;
}

bool SDCard::getFirstLine(SdFat *sd, char *fname, char *line)
{
    char linetoread[64];
    FsFile file = sd->open(fname, O_RDONLY);
    file.rewind();

    while (file.available())
    {
        int n = file.fgets(linetoread, 64);
        if (n <= 0)
        {
            // Serial.println("[WARN] Read failed");
            file.close();
            return false;
        }
        if (linetoread[n - 1] != '\n' && n == (sizeof(linetoread) - 1))
        {
            // Serial.println("[WARN] Line too long");
            file.close();
            return false;
        }
        strcpy(line, linetoread);
    }

    file.close();
    return true;
}

bool SDCard::deleteFile(SdFat *sd, char *fname)
{
    if (!sd->remove(fname))
    {
        // sd->errorHalt(&Serial);
        // Serial.println("[WARN] failed to remove");
    }
}

int SDCard::last_miso = 0;
int SDCard::last_mosi = 0;

void SDCard::attachInterrupts()
{
    attachInterrupt(
        MISO_PIN, []()
        { sdcard.last_miso = millis(); },
        RISING);
    attachInterrupt(
        MOSI_PIN, []()
        { sdcard.last_mosi = millis(); },
        RISING);
}

void SDCard::takeControl()
{
    pinMode(MISO_PIN, SPECIAL);
    pinMode(MOSI_PIN, SPECIAL);
    pinMode(SCLK_PIN, SPECIAL);
    pinMode(SD_CS_PIN, OUTPUT);
}

// ------------------------
void SDCard::giveControl()
{
    // ------------------------
    pinMode(MISO_PIN, INPUT);
    pinMode(MOSI_PIN, INPUT);
    pinMode(SCLK_PIN, INPUT);
    pinMode(SD_CS_PIN, INPUT);
}