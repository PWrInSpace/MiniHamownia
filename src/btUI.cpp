#include "btUI.h"

void BluetoothUI::begin(String name){
    BTSerial.begin(name);
    EEPROM.begin(DATA_SIZE);
    this->readFlash();
}

bool BluetoothUI::isConnected(){
    return BTSerial.hasClient();
}

void BluetoothUI::println(const String & message){
    BTSerial.println(message);
}

bool BluetoothUI::available(){
    return BTSerial.available();
}

String BluetoothUI::readString(){
    String result = "";
    char value;
    while(BTSerial.available()){
        value = BTSerial.read();
        
        if(value != '\n'){
            result += value;
        }
    }

    return result;
}

void BluetoothUI::printTimers(){
    BTSerial.print("Calibration Factor: ");
    BTSerial.println(calibrationFactor);
    BTSerial.print("Count down: ");
    BTSerial.println(countDownTime);

    BTSerial.print("Valve 1: ");
    firstValveEnable > 0 ? BTSerial.println("Enable"): BTSerial.println("Disable");
    BTSerial.print("Open time: ");
    BTSerial.println(firstValveOpenTime);
    BTSerial.print("Close time: ");
    BTSerial.println(firstValveCloseTime);

    BTSerial.print("Valve 2: ");
    secondValveEnable > 0 ? BTSerial.println("Enable"): BTSerial.println("Disable");
    BTSerial.print("Open time: ");
    BTSerial.println(secondValveOpenTime);
    BTSerial.print("Close time: ");
    BTSerial.println(secondValveCloseTime);

}

void BluetoothUI::saveToFlash(){
    uint8_t flashFrame[DATA_SIZE] = {0};
    uint8_t elem=0;

    flashFrame[elem++] |= calibrationFactor >> 8;
    flashFrame[elem++] |= calibrationFactor;
    
    flashFrame[elem++] |= countDownTime >> 8;
    flashFrame[elem++] |= countDownTime;
    
    flashFrame[elem++] |= firstValveOpenTime >> 24;
    flashFrame[elem++] |= firstValveOpenTime >> 16;
    flashFrame[elem++] |= firstValveOpenTime >> 8;
    flashFrame[elem++] |= firstValveOpenTime;
    flashFrame[elem++] |= firstValveCloseTime >> 24;
    flashFrame[elem++] |= firstValveCloseTime >> 16;
    flashFrame[elem++] |= firstValveCloseTime >> 8;
    flashFrame[elem++] |= firstValveCloseTime;
    flashFrame[elem++] |= firstValveEnable;
    
    flashFrame[elem++] |= secondValveOpenTime >> 24;
    flashFrame[elem++] |= secondValveOpenTime >> 16;
    flashFrame[elem++] |= secondValveOpenTime >> 8;
    flashFrame[elem++] |= secondValveOpenTime;
    flashFrame[elem++] |= secondValveCloseTime >> 24;
    flashFrame[elem++] |= secondValveCloseTime >> 16;
    flashFrame[elem++] |= secondValveCloseTime >> 8;
    flashFrame[elem++] |= secondValveCloseTime;
    flashFrame[elem++] |= secondValveEnable;

    //... add more data to Frame, firstly change DATA_SIZE

    for(int i = 0; i < DATA_SIZE; ++i){
        EEPROM.write(i, flashFrame[i]);
    }

    EEPROM.commit();
}

void BluetoothUI::readFlash(){
    uint8_t flashFrame[DATA_SIZE];
    uint8_t elem = 0;
    
    for(int i = 0; i < DATA_SIZE ; ++i){
        flashFrame[i] = EEPROM.read(i);
    }

    calibrationFactor |= flashFrame[elem++] << 8;
    calibrationFactor |= flashFrame[elem++];

    countDownTime |= flashFrame[elem++] << 8;
    countDownTime |= flashFrame[elem++];

    
    firstValveOpenTime |= flashFrame[elem++] << 24;
    firstValveOpenTime |= flashFrame[elem++] << 16;
    firstValveOpenTime |= flashFrame[elem++] << 8;
    firstValveOpenTime |= flashFrame[elem++];
    firstValveCloseTime |= flashFrame[elem++] << 24;
    firstValveCloseTime |= flashFrame[elem++] << 16;
    firstValveCloseTime |= flashFrame[elem++] << 8;
    firstValveCloseTime |= flashFrame[elem++];
    firstValveEnable |= flashFrame[elem++];

    secondValveOpenTime |= flashFrame[elem++] << 24;
    secondValveOpenTime |= flashFrame[elem++] << 16;
    secondValveOpenTime |= flashFrame[elem++] << 8;
    secondValveOpenTime |= flashFrame[elem++];
    secondValveCloseTime |= flashFrame[elem++] << 24;
    secondValveCloseTime |= flashFrame[elem++] << 16;
    secondValveCloseTime |= flashFrame[elem++] << 8;
    secondValveCloseTime |= flashFrame[elem++];
    secondValveEnable |= flashFrame[elem++];
}

void BluetoothUI::setCalibrationFactor(uint16_t cf){
    calibrationFactor = cf;
    //saveToFlash();  //not the best solution, but the most user friendly;
}

uint16_t BluetoothUI::getCalibrationFactor() const{
    return calibrationFactor;
}

bool BluetoothUI::setCountDownTime(uint16_t time){
    if(time >= 10000){
        countDownTime = time;
        return true;
    }

    return false;
}

uint16_t BluetoothUI::getCountDownTime() const{
    return countDownTime;
}

bool BluetoothUI::setValveOpenTimer(uint32_t time, uint8_t valve){
    switch(valve){
        case 1:
            firstValveOpenTime = time;
            break;
        case 2:
            secondValveOpenTime = time;
            break;
        default:
            return false;
    }

    return true;
}

uint32_t BluetoothUI::getValveOpenTimer(uint8_t valve){
    uint32_t result = 0;
    
    switch(valve){
        case 1:
            result = firstValveOpenTime;
            break;
        case 2:
            result = secondValveOpenTime;
            break;
    }

    return result;
}

bool BluetoothUI::setValveCloseTimer(uint32_t time, uint8_t valve){
    switch(valve){
        case 1:
            firstValveCloseTime = time;
            break;
        case 2:
            secondValveCloseTime = time;
            break;
        default:
            return false;
    }

    return true;
}

uint32_t BluetoothUI::getValveCloseTimer(uint8_t valve){
    uint32_t result = 0;
    
    switch(valve){
        case 1:
            result = firstValveCloseTime;
            break;
        case 2:
            result = secondValveCloseTime;
            break;
    }

    return result;
}

bool BluetoothUI::setValveState(uint8_t state, uint8_t valve){
    if(state > 1) state = 1;

    switch(valve){
        case 1:
            firstValveEnable = state;
            break;
        case 2:
            secondValveEnable = state;
            break;
        default:
            return false;
    }
    
    return true;
}

uint8_t BluetoothUI::getValveState(uint8_t valve){
    uint8_t result = 0;
    
    switch(valve){
        case 1:
            result = firstValveEnable;
            break;
        case 2:
            result = secondValveEnable;
            break;
    }

    return result;
}

/*
void BluetoothUI::setval(){
    calibrationFactor = 1000;
    countDownTime = 10000;
    firstValveOpenTime = 15000;
    firstValveCloseTime = 20000;
    firstValveEnable = 1;
    secondValveOpenTime = 20000;
    secondValveCloseTime = 25000;
    secondValveEnable = 0;
}*/


bool checkCommand(String msg, String prefix, char delimiter, uint8_t delimiter_num){
    uint8_t msgLength = msg.length();
    uint8_t counter = 0;

    if(!msg.startsWith(prefix)) return false;

    for(int i = 0; i < msgLength; ++i){
        if(msg[i] == delimiter) counter++;
    }

    if(counter != delimiter_num) return false;

    return true;
}



