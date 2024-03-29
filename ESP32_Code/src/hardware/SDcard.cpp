#include "../include/hardware/SDcard.h"


SDCard::SDCard(SPIClass *_spi, uint8_t _cs):
    spi(_spi),  cs(_cs){};

bool SDCard::init(){
    //spi = SPIClass(HSPI);
    //spi.begin(sck, miso, mosi, cs);
    //SPI.setClockDivider(SPI_CLOCK_DIV2);
    
    if(!SD.begin(cs, *spi)){ //idk xD
        return false;
    }

    return true;
}

bool SDCard::write(String path, const String & dataFrame){
    File file = SD.open(path, "a");  
    
    if(file) {
        if(!file.write((uint8_t *) dataFrame.c_str(), dataFrame.length())) {
            file.close();
            return false;
        }
    }else {
        return false;
    }
    
    file.close();

    return true;
}

bool SDCard::fileExists(String path)
{
    return SD.exists(path);
}