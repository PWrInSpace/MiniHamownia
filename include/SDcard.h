#ifndef SDCARD_H
#define SDCARD_H

#include <FS.h>
#include <SPI.h>
#include <SD.h>


class SDCard{
    uint8_t mosi;
    uint8_t miso; 
    uint8_t sck;
    uint8_t cs;
    SPIClass spi;

    public:
    SDCard(uint8_t _mosi, uint8_t miso, uint8_t _sck, uint8_t _cs);
    bool init();
    bool write(String path, const String & data); 
    bool fileExists(String path);
};

#endif