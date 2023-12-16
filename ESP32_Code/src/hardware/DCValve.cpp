#include "../include/hardware/DCValve.h" 
#include "btUI.h"

extern BluetoothUI btUI;

DCValve::DCValve(uint8_t _motor1, uint8_t _motor2, uint8_t _pwm, uint8_t _lsp1, uint8_t _lsp2, uint16_t _freq, uint8_t _res, uint8_t _chan)
    :motorPin1(_motor1),
    motorPin2(_motor2),
    pwmPin(_pwm),
    limitSwitchPin1(_lsp1),
    limitSwitchPin2(_lsp2),
    frequency(_freq),
    resolution(_res),
    pwmChannel(_chan),
    timeout(2000) 
{}

// void DCValve::setTimeout(uint16_t newTimeout){
//     timeout = newTimeout;
// }

void DCValve::init(){
    //DC mototr setup
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(pwmPin, OUTPUT);
    //limit switchs
    pinMode(limitSwitchPin1, INPUT_PULLUP);
    pinMode(limitSwitchPin2, INPUT_PULLUP);

    //pwm setup
    ledcSetup(pwmChannel, frequency, resolution);
    ledcAttachPin(pwmPin, pwmChannel);
    //Serial.println("Valve init complete");
    
    //turn off motor
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    ledcWrite(pwmChannel, 0);
}

void DCValve::valveMove(const uint8_t & limitSwitchPIN, const uint8_t & highValvePIN, const uint8_t & valveSpeed){
    if(!digitalRead(limitSwitchPIN)){ // gdy krancówka jest zwarta
        //BTSerial.println("Na krancowce: " + String(limitSwitchPIN));
        return;
    }

    //turn on dc
    digitalWrite(highValvePIN, HIGH);
    ledcWrite(pwmChannel, valveSpeed);
    
    //max work time
    uint16_t timeoutValve = timeout;

    while(digitalRead(limitSwitchPIN) && timeoutValve > 0) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
        timeoutValve--;
    }

    //turn off dc
    digitalWrite(highValvePIN, LOW);
    ledcWrite(pwmChannel, 0);
}


void DCValve::open(){
    valveMove(limitSwitchPin2, motorPin1);
}


void DCValve::close(){
    valveMove(limitSwitchPin1, motorPin2);
}


void DCValve::timeOpen(uint32_t time){
    uint32_t openTime = time;
    uint32_t valveTimer;

    this->open();

    valveTimer = millis();
    while(millis() - valveTimer < openTime){
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    
    this->close();
}

int8_t DCValve::getPosition()
{
    if(!digitalRead(limitSwitchPin1))
    {
        return 0; //Valve closed
    }
    else if(!digitalRead(limitSwitchPin2))
    {
        return 1; //Valve open
    }
    else return 2; //Valve moving
}


DCValve firstValve(DCIN1, DCIN2, DC_PWM1, LIM_SW_1, LIM_SW_2);
DCValve secondValve(DCIN1, DCIN2, DC_PWM1, LIM_SW_1, LIM_SW_2);


void openFirstValve(void *arg){
    firstValve.open();
    vTaskDelete(NULL);
}

void openSecondValve(void *arg){
    secondValve.open();
    vTaskDelete(NULL);
}

void closeFirstValve(void *arg){
    firstValve.close();
    vTaskDelete(NULL);
}

void closeSecondValve(void *arg){
    secondValve.close();
    vTaskDelete(NULL);
}

void timeOpenFirstValve(void *arg){
    uint32_t time = *(uint32_t*)arg;

    firstValve.timeOpen(time);
    vTaskDelete(NULL);
}

void timeOpenSecondValve(void *arg){
    uint32_t time = *(uint32_t*)arg;

    secondValve.timeOpen(time);
    vTaskDelete(NULL);
}

