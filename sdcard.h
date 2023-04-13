#include "SdFat.h"

#ifndef _SDCARD_H_
#define _SDCARD_H_

#define READ_TIMEOUT 10000UL

class SDCard
{
public:
    static void attachInterrupts();
    static SdFat *initialise();
    static char *getCsvFile(SdFat *sd);
    static char *getFirstLine(SdFat *sd, char *fname);
    static char *deleteFile(SdFat *sd, char *fname);
    static void takeControl();
    static void giveControl();
    static int last_miso;
    static int last_mosi;
};

extern SDCard sdcard;

#endif