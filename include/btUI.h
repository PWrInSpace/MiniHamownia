#ifndef BLUETOOTH_UI_HH
#define BLUETOOTH_UI_HH

class BluetoothUI{
    uint16_t firstMotorTimer;
    uint16_t secondMotorTimer;
    uint16_t igniterTimer;
    uint16_t countDown;
    
    public:
    BluetoothUI();
    bool begin(); //read from flash
    void setMotorTimer(uint16_t time, uint8_t motor);  //write to flash
    uint16_t getMotorTimer(uint8_t motor);
    void setIgniterTimer(uint16_t timer);
    uint16_t getIgniterTimer();
    void setCountDownTime();
    uint16_t getCountDownTime();
    bool checkTimers();  //check timers sequence 

    private:
    String createTimersFrame();
    void getTimeersFromFrame(String frame);
    String readFlash();
    void writeFlash(char* data);
};

#endif