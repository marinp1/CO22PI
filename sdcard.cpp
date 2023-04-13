#include "sdcard.h"
#include "pins.h"

#define SPI_CLOCK SD_SCK_MHZ(4)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)

SdFat *SDCard::initialise()
{
    static SdFat sd;
    if (!sd.begin(SD_CS_PIN, SPI_CLOCK))
    {
        sd.errorPrint(&Serial);
        Serial.println("[WARN] Failed to init sd");
        return 0;
    }
    return &sd;
};

static char csv_ext[5] = ".csv";

char *SDCard::getCsvFile(SdFat *sd)
{
    File32 file, dir;
    char fileName[64];

    if (!dir.open("/"))
    {
        Serial.println("[WARN] Failed to open root dir");
    }
    else
    {
        while (file.openNext(&dir, O_READ))
        {
            if (!file.isHidden())
            {
                if (!file.isDir())
                {
                    file.getName(fileName, sizeof(fileName));
                    Serial.println(fileName);
                }
            }
            file.close();
        }
    }
}

char *SDCard::getFirstLine(SdFat *sd, char *fname)
{
    static char line[128] = "0";

    FsFile file = sd->open(fname, O_RDONLY);
    int linetoread = 1;
    while (linetoread-- > 0)
    {
        int n = file.fgets(line, 128);
        if (n <= 0)
        {
            Serial.println("[WARN] fgets failed");
            file.close();
            return 0;
        }
    }

    file.close();

    if (line == "0")
    {
        return 0;
    }

    return line;
}

char *SDCard::deleteFile(SdFat *sd, char *fname)
{
    if (!sd->remove(fname))
    {
        sd->errorHalt("failed to remove");
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