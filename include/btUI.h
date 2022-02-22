#ifndef BLUETOOTH_UI_HH
#define BLUETOOTH_UI_HH

#include "BluetoothSerial.h"
#include "EEPROM.h"
#define DATA_SIZE 24 //24 bytes of class data
#define FIRST_VALVE 1
#define SECOND_VALVE 2


class BluetoothUI{
    uint16_t calibrationFactor; 
    uint16_t pressureSensCalibrationFactor;
    uint16_t countDownTime;
    uint32_t firstValveOpenTime;
    uint32_t firstValveCloseTime;
    uint8_t firstValveEnable;
    uint32_t secondValveOpenTime;
    uint32_t secondValveCloseTime;
    uint8_t secondValveEnable;
    //16 first bits of flash memory
    BluetoothSerial BTSerial;
    bool btDataFlag;
    
    public:
    BluetoothUI() = default;
    void begin(String name = "rozkurwiacz"); //read from flash
    
    void setCalibrationFactor(uint16_t cf);
    uint16_t getCalibrationFactor() const;
    void setPressureSensCalibrationFactor(uint16_t cf);
    uint16_t getPressureSensCalibrationFactor() const;
    bool setCountDownTime(uint16_t time);
    uint16_t getCountDownTime() const;
    bool setValveOpenTimer(uint32_t time, uint8_t valve); 
    uint32_t getValveOpenTimer(uint8_t valve);
    bool setValveCloseTimer(uint32_t time, uint8_t valve);
    uint32_t getValveCloseTimer(uint8_t valve);
    bool setValveState(uint8_t state, uint8_t valve);
    uint8_t getValveState(uint8_t valve);

    bool checkTimers();  //check timers sequence 
    bool checkDataFlag();
    bool switchDataFlag();
    void println(const String & message);
    bool isConnected();
    String timersDescription();
    bool available();
    
    String readString();
    //void setval();


    void saveToFlash();
    private:
    void readFlash();
};

bool checkCommand(String msg, String prefix, char delimeter, uint8_t delimeter_num = 1);


#endif