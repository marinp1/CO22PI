#ifndef _SD_H_
#define _SD_H_

#define LED_PIN 2
#define SD_CS_PIN 4
#define MISO_PIN 12 // RISING received when reading data
#define MOSI_PIN 13 // RISING received when writing data
#define SCLK_PIN 14
#define CS_SENSE 5

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_HALF_SPEED)

#define INIT_LED                  \
    {                             \
        pinMode(LED_PIN, OUTPUT); \
    }

#define LED_ON                      \
    {                               \
        digitalWrite(LED_PIN, LOW); \
    }

#define LED_OFF                      \
    {                                \
        digitalWrite(LED_PIN, HIGH); \
    }

#define GIVE_SPI_CONTROL           \
    {                              \
        pinMode(MISO_PIN, INPUT);  \
        pinMode(MOSI_PIN, INPUT);  \
        pinMode(SCLK_PIN, INPUT);  \
        pinMode(SD_CS_PIN, INPUT); \
    }

#endif