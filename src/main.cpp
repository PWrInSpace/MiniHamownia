#include "Arduino.h"
/*********************************************************
* LEDY
* 2 - ERROR_LED
* 2 - STATUS_LED
*
* KARTA mySD
* 33 - CS
* 25 - MOSI
* 27 - MISO
* 26 - SCK
* 
* HX 711
* 26 - CLK / SCK
* 15 - DOUT / DT
* 
* SOFTWARE BTSerial
* 
* 
* 19 - IGNITER
* 34 - IGNITER CONTINUITY 
* 
* 22 - DC IN2
* 21 - DC IN1
* 
* 13 - LIMIT_SW 1
* 12 - LIMIT_SW 2
* 
* 36 (SVN) - PRESS_SENS
* 
* A0 - RANDOM_SEED
*********************************************************/
#include "HX711_ADC.h"
#include <SPI.h>
#include "SDcard.h"
#include "BluetoothSerial.h"
#include "DCValve.h"
//#include <SoftwareSerial.h>

#define DOUT 15 //pin 3 Arduino i wyjście DAT czujnika
#define CLK 26  //pin 2 Arduino i wyjście CLK czujnika
#define ERROR_LED 2
#define STATUS_LED 2

// igniter
#define CONTINUITY 34
#define IGNITER 19

// DC motors
#define DCIN1 22
#define DCIN2 21
#define LIM_SW_1 12
#define LIM_SW_2 13

// PRESSURE SENS
#define PRESS_SENS 36

// mySD
#define MOSI 27
#define MISO 25
#define SCK 26
#define CS 33

// VALVE STATES
#define VALVE_OPEN 2
#define VALVE_BETWEEN 1
#define VALVE_CLOSE 0

BluetoothSerial BTSerial;
DCValve Valve(DCIN1, DCIN2, 69, LIM_SW_2, LIM_SW_1);
HX711_ADC LoadCell(DOUT, CLK);
SDCard sdCard(27, 25, 26, 33);
//SoftwareSerial myserial(5, 6);  //RX TX

int dlug;
String tab;
String wartosc;

bool start = false;
char x = '0';             //znak wczytany z terminala - do ruszenia zapisu na mySD
float data;         //Dane z belki
String data_string; //String przechowujący jeden rzad danych
String file_name;   //nazwa pliku

uint32_t sd_time = 0; //Czas dla zapisu na mySD

float pressureConstant = 1.0;

//Belka
bool _tare = true;
const float calibration_factor = 605.88; //Odczytana wartość z programu kalibracyjnego
unsigned long stabilizingTime = 2000;    //opóźnienie uruchomienia belki
int iter = 0;
bool igniterBool = false;
bool igniterOffBool = false;
bool valveOpenBool = false;
bool valveCloseBool = false;
uint32_t igniterDelay = 10000;
uint32_t valveOpenDelay = igniterDelay + 10000;
uint32_t valveCloseDelay = valveOpenDelay + 2300;

uint8_t valveState(uint8_t limit_sw1, uint8_t limit_sw2)
{
  bool open = digitalRead(limit_sw1);
  bool close = digitalRead(limit_sw2);

  if (open && close)
    return VALVE_BETWEEN;
  else if (!open && close)
    return VALVE_OPEN;
  else if (open && !close)
    return VALVE_CLOSE;
  else
    return VALVE_BETWEEN;
}

void setup()
{
  BTSerial.begin("rozkurwiacz");
  //Serial.begin(115200);
  // LED
  pinMode(ERROR_LED, OUTPUT); //Led error
  digitalWrite(ERROR_LED, LOW);
  pinMode(STATUS_LED, OUTPUT); //Status error
  digitalWrite(STATUS_LED, LOW);

  // DC motors
  pinMode(LIM_SW_1, INPUT);
  pinMode(LIM_SW_2, INPUT);

  pinMode(DCIN1, OUTPUT);
  pinMode(DCIN2, OUTPUT);

  // igniter
  pinMode(CONTINUITY, INPUT);
  pinMode(IGNITER, OUTPUT);
  digitalWrite(IGNITER, LOW);
  // press sens
  pinMode(PRESS_SENS, INPUT);

  Valve.init();

  randomSeed(analogRead(A0));
  //valveCloseBool = true;

  if (LoadCell.getTareTimeoutFlag())
  { //sprawdzenie czy jakieś dane przychodzą z belki
    BTSerial.println("Brak podlaczonej belki\n");
    while (true)
    {
      BTSerial.println("Brak podlaczonej belki\n");
      delay(500);
    }
  }

  LoadCell.setCalFactor(calibration_factor);
  LoadCell.setSamplesInUse(1);

  //Inicjalizacja karty mySD
  
  if(!sdCard.init()){
    digitalWrite(ERROR_LED, HIGH);
    while(true){
      BTSerial.println("SD error");
      delay(500);
    }
  }
  Valve.valveClose((void *)'c');
}

void loop()
{
  
  if (analogRead(CONTINUITY) > 2000)
    digitalWrite(STATUS_LED, HIGH);
  else
    digitalWrite(STATUS_LED, LOW);

  data_string = "";

  //Sterowanie zapisem na karte poprzez terminal
  if (BTSerial.hasClient() > 0)
  {
    digitalWrite(STATUS_LED, HIGH);
    x = BTSerial.read();
    if (x == '2')
    {
      start = false;
    }
    else if (x == '1')
    {
      if (analogRead(CONTINUITY) < 2000)
      {
        while (true)
        {
          BTSerial.println("Brak ciaglosci zapalnika");
          delay(500);
        }
      }
      start = true;
      //sd_create();
      digitalWrite(STATUS_LED, start);
      sd_time = millis();
      x = 0;
    }
  }

  data_string += String(millis() - sd_time) + "; ";

  //odczyt danych
  if (LoadCell.update())
  {
    data = LoadCell.getData(); //* 9.81;
    //if(data < 0.5) data = 0;
    //data = data * 9.81;
    // save force
    data_string += String(data) + "; ";
  }

  // save pressure
  data_string += String(analogRead(PRESS_SENS) * pressureConstant) + "; ";

  if (start)
  {
    if ((millis() - sd_time > igniterDelay) && !igniterBool)
    {
      BTSerial.println("ogein");
      digitalWrite(IGNITER, HIGH);
      igniterBool = true;
    }
    if ((millis() - sd_time > igniterDelay + 1000) && !igniterOffBool)
    {
      BTSerial.println("noogein");
      digitalWrite(IGNITER, LOW);
      igniterOffBool = true;
    }
    if ((millis() - sd_time > valveOpenDelay) && !valveOpenBool)
    {
      BTSerial.println("valve_open");
      Valve.valveOpen((void *)'c');
      valveOpenBool = true;
    }
    if ((millis() - sd_time > valveCloseDelay) && !valveCloseBool)
    {
      BTSerial.println("valve_close");
      Valve.valveClose((void *)'c');
      valveCloseBool = true;
    }
  }
  data_string += String(valveState(LIM_SW_1, LIM_SW_2)) + "; " + String(igniterBool) + "; ";
  
  if(start){
    sdCard.write("/teeeest.txt", data_string + "\n");
  }       

  if ((iter++) == 100){
    iter = 0;
    BTSerial.println(data_string); //wysyła dane po uarcie
  }
  
}
