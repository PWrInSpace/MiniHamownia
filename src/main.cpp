#include "Arduino.h"

#include "HX711_ADC.h"
#include <SPI.h>
#include "SDcard.h"
#include "BluetoothSerial.h"
#include "DCValve.h"
#include "pinout.h"

BluetoothSerial BTSerial;
DCValve Valve(DCIN1, DCIN2, 69, LIM_SW_2, LIM_SW_1);
HX711_ADC LoadCell(DOUT, CLK);
SDCard sdCard(27, 25, 26, 33);

void setup()
{
  

}


void loop()
{

}
