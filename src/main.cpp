#include "Arduino.h"
#include "btUI.h"
#include "BluetoothSerial.h"

BluetoothUI btui;

bool led = false;

void setup()
{
    Serial.begin(115200);
    pinMode(2, OUTPUT);
    btui.begin();

    btui.printTimers();
    /*
    while(!btui.isConnected()){
        led = !led;
        digitalWrite(2, led);
        delay(500);
    }

    btui.println("Connected!");
    digitalWrite(2, HIGH);
    */
}


void loop()
{
    if(btui.available()){
        btui.println(btui.readString());
    }
    delay(1000);
}
