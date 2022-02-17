#ifndef SDCARD_H
#define SDCARD_H

#include <FS.h>
#include <SPI.h>
#include <SD.h>


class SDCard{
    SPIClass spi;
    uint8_t cs;

    public:
    SDCard(SPIClass &_spi, uint8_t _cs);
    bool init();
    bool write(String path, const String & data); 
};

#endif