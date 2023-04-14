#include "SdFat.h"

#ifndef _SDCARD_H_
#define _SDCARD_H_

#define READ_TIMEOUT 10000UL

class SDCard
{
public:
    static void attachInterrupts();
    static bool initialise(SdFat *sd);
    static bool getCsvFile(SdFat *sd, char *latest_fname);
    static bool getFirstLine(SdFat *sd, char *fname, char *line);
    static char *deleteFile(SdFat *sd, char *fname);
    static void takeControl();
    static void giveControl();
    static int last_miso;
    static int last_mosi;
};

extern SDCard sdcard;

#endif