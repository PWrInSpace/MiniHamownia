#include "DCValve.h" 
#include "BluetoothSerial.h"

extern BluetoothSerial BTSerial;

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

void DCValve::setTimeout(uint16_t newTimeout){
    timeout = newTimeout;
}

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
        BTSerial.println("Na krancowce: " + String(limitSwitchPIN));
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

    //wylaczenie silnika
    digitalWrite(highValvePIN, LOW);
    ledcWrite(pwmChannel, 0);
}


void DCValve::valveOpen(void *arg){
    valveMove(limitSwitchPin2, motorPin1);
    
    //vTaskDelete(NULL);
    BTSerial.println("Koniec taska");
}


void DCValve::valveClose(void *arg){
    valveMove(limitSwitchPin1, motorPin2);
    
    //vTaskDelete(NULL);
}


void DCValve::valveTimeOpen(void *arg){

    uint16_t openTime = 500;
    uint16_t valveTimer;

    valveMove(limitSwitchPin2, motorPin1);

    valveTimer = millis();
    while(millis() - valveTimer < openTime){
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    
    valveMove(limitSwitchPin2, motorPin2);
    vTaskDelete(NULL);
}
