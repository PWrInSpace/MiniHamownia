#ifndef BLUETOOTH_UI_HH
#define BLUETOOTH_UI_HH

#include "BluetoothSerial.h"
#include "EEPROM.h"

#define DATA_SIZE 30 // 30 bytes of class data
#define FIRST_VALVE 1
#define SECOND_VALVE 2

#define FIRST_LOAD_CELL 1
#define SECOND_LOAD_CELL 2

class BluetoothUI
{
    uint16_t firstLoadCellCalibrationFactor;
    uint16_t secondLoadCellCalibrationFactor;
    uint16_t pressureSensCalibrationFactor;
    uint16_t countDownTime;
    uint32_t testTime;
    uint32_t firstValveOpenTime;
    uint32_t firstValveCloseTime;
    uint8_t firstValveEnable;
    uint32_t secondValveOpenTime;
    uint32_t secondValveCloseTime;
    uint8_t secondValveEnable;
    // 16 first bits of flash memory
    BluetoothSerial BTSerial;
    bool btDataFlag;
    bool ctFlag;
    bool btCheckCalibrationFlag;
    bool btTareFlag[2];

public:
    BluetoothUI() = default;
    void begin(String name = "rozkurwiacz"); // read from flash

    bool setCalibrationFactor(uint16_t cf, uint8_t loadCellIndex);
    uint16_t getCalibrationFactor(uint8_t loadCellFactor) const;
    void setPressureSensCalibrationFactor(uint16_t cf);
    uint16_t getPressureSensCalibrationFactor() const;
    bool setCountDownTime(uint16_t time);
    uint16_t getCountDownTime() const;
    bool setTestTime(uint32_t time);
    uint32_t getTestTime() const;   
    bool setValveOpenTimer(uint32_t time, uint8_t valve);
    uint32_t getValveOpenTimer(uint8_t valve);
    bool setValveCloseTimer(uint32_t time, uint8_t valve);
    uint32_t getValveCloseTimer(uint8_t valve);
    bool setValveState(uint8_t state, uint8_t valve);
    uint8_t getValveState(uint8_t valve);

    bool checkTimers(); // check timers sequence
    bool checkBtFlag();
    bool switchBtFlag();
    bool checkCTFlag(); //Checks if continuity override is applied
    bool switchCtFlag();
    bool checkCalibrationFactorsFlag();
    bool switchCalibrationFactorsFlag();
    bool checkTareFlag(bool cell);
    bool switchTareFlag(bool cell);
    void println(const String &message);
    bool isConnected();
    String timersDescription();
    bool available();

    String readString();
    // void setval();

    void saveToFlash();

private:
    void readFlash();
};

bool checkCommand(String msg, String prefix, char delimeter, uint8_t delimeter_num = 1);

#endif