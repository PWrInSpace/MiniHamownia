#include "loopTasks.h"
#include "BluetoothSerial.h"
#include "btUI.h"
#include "stateMachine.h"
#include "SDcard.h"
#include "pinout.h"
#include "DCValve.h"
#include "HX711_ADC.h"
#include "trafag8252.h"
#include "ESP32Servo.h"

extern BluetoothUI btUI;
extern StateMachine sm;
extern DCValve firstValve;
extern DCValve secondValve;
extern SPIClass myspi;
extern Servo servo;

//***********************************//
//             PRO_CPU               //
//***********************************//

void btReceiveTask(void *arg)
{
  // pinMode(BUZZER, OUTPUT); //move to pinout.h
  String msg;

  while (true)
  {
    if (btUI.isConnected())
    {
      digitalWrite(2, LOW);

      if (btUI.available())
      {
        msg = btUI.readString();
        xQueueSend(sm.btRxQueue, (void *)&msg, 10); // TO DO error handling
      }
    }
    else
    {
      // if(sm.state != DISCONNECTED){
      digitalWrite(2, HIGH);
      if (sm.state < COUNTDOWN)
      {
        sm.changeState(DISCONNECTED);
      }
      else
      {
        msg = "LOG: Disconnected in state: " + String(sm.state) + "\n";
        xQueueSend(sm.sdQueue, (void *)&msg, 0);
      }

      // disconnected sound
      beepBoop(300, 6);

      while (!btUI.isConnected())
      {
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      // connected sound
      beepBoop(300, 2);
      if (sm.state < COUNTDOWN)
      {
        sm.changeState(IDLE);
      }

      msg = "State: " + String(sm.state);
      xQueueSend(sm.btTxQueue, (void *)&msg, 10);
      //}
    }
    
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void btTransmitTask(void *arg)
{
  String btMsg;

  while (1)
  {
    xQueueReceive(sm.btTxQueue, (void *)&btMsg, portMAX_DELAY); // wait until data appear in queue
    if (btUI.isConnected())
    {

      btUI.println(btMsg);
    }
    else
    {
      // error
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void uiTask(void *arg)
{
  String btMsg;
  String prefix = "MH;"; // prefix
  String command;
  uint32_t time;
  String btTx;
  TickType_t askTime = 0;
  TickType_t askTimeOut = 30000;
  
  servo.attach(SERVO_PIN, 500, 2400);

  while (1)
  {
    if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, 1000) == pdTRUE)
    {
      if (sm.state == COUNTDOWN)
      { // message in countdown state

        sm.changeState(ABORT);

        // frame check
      }
      else if (checkCommand(btMsg, prefix, ';', 2) && (sm.state == IDLE))
      {
        btMsg.remove(0, prefix.length());  // remove MH; prefix
        command = btMsg.substring(0, 4);   // get command
        time = btMsg.substring(4).toInt(); // get timer ms

        // valve open timer
        if (command == "MO1;")
        {
          if (command[2] == '1')
          {
            //xTaskCreatePinnedToCore(openFirstValve, "First Val open", 2048, NULL, 2, NULL, APP_CPU_NUM);
            servo.write(SERVO_OPEN_POSITION);
            btTx = "Servo open";
          }
          else if (command[2] == '2')
          {
            xTaskCreatePinnedToCore(openSecondValve, "second Val open", 2048, NULL, 2, NULL, APP_CPU_NUM);
            btTx = "Second valve open";
          }
          else
          {
            btTx = "Unknown valve number";
          }
        }
        else if (command == "MC1;")
        {
          if (command[2] == '1')
          {
            //xTaskCreatePinnedToCore(closeFirstValve, "First Val close", 2048, NULL, 2, NULL, APP_CPU_NUM);
            servo.write(SERVO_CLOSE_POSITION);
            btTx = "Servo closed";
          }
          else if (command[2] == '2')
          {
            xTaskCreatePinnedToCore(closeSecondValve, "second Val close", 2048, NULL, 2, NULL, APP_CPU_NUM);
            btTx = "Second valve closed";
          }
          else
          {
            btTx = "Unknown valve number";
          }
        }
        else if (command == "TO1;")
        {
          if (command[2] == '1')
          { 
            servo.write(time);
            //xTaskCreatePinnedToCore(timeOpenFirstValve, "First val time open", 2048, (void *)&time, 2, NULL, APP_CPU_NUM); // i think it won't work xDD
            btTx = "Servo move: " + String(time);
          }
          else if (command[2] == '2')
          {
            xTaskCreatePinnedToCore(timeOpenSecondValve, "Second val time open", 2048, (void *)&time, 2, NULL, APP_CPU_NUM);
            btTx = "First valve open for " + String(time);
          }
          else
          {
            btTx = "Unknown valve number";
          }
        }
        else if (command == "VO1;")
        {
          if (btUI.setValveOpenTimer(time, command[2] - 48))
          {
            btTx = "Valve " + String(command[2] - 48) + " New open time: " + String(btUI.getValveOpenTimer(command[2] - 48));
          }
          else
          {
            btTx = "Failed to save";
          }

          // valve close timer
        }
        else if (command == "VC1;")
        {
          if (btUI.setValveCloseTimer(time, command[2] - 48))
          {
            btTx = "Valve " + String(command[2] - 48) + " New close time: " + String(btUI.getValveCloseTimer(command[2] - 48));
          }
          else
          {
            btTx = "Failed to save";
          }

          // valve enable
        }
        else if (command == "VE1;")
        {
          if (btUI.setValveState(time, command[2] - 48))
          {
            btTx = "Valve " + String(command[2] - 48) + (btUI.getValveState(command[2] - 48) > 0 ? " enable" : " disable");
          }
          else
          {
            btTx = "Failed to save";
          }

          // count down timer
        }
        else if (command == "CDT;")
        {
          if (btUI.setCountDownTime(time))
          {
            btTx = "New countdown time:  " + String(btUI.getCountDownTime());
          }
          else
          {
            btTx = "Failed to save";
          }
        }
        else if (command == "JP1;")
        {
          btUI.setCalibrationFactor(time, FIRST_LOAD_CELL); // value not time :D
          btTx = "New calibration factor:  " + String(btUI.getCalibrationFactor(FIRST_LOAD_CELL));
        }
        else if (command == "JP2;")
        {
          btUI.setCalibrationFactor(time, SECOND_LOAD_CELL); // value not time :D
          btTx = "New calibration factor:  " + String(btUI.getCalibrationFactor(SECOND_LOAD_CELL));
        }
        else if (command == "JP3;")
        {
          btUI.setPressureSensCalibrationFactor(time); // value not time :)
          btTx = "New pressure sens calibration factor:  " + String(btUI.getPressureSensCalibrationFactor());
        }
        else if (command == "LC1;" || command == "LC2;" || command == "PSC;")
        {
          if (xQueueSend(sm.btRxQueue, (void *)&command, 0) == pdTRUE)
          {
            sm.changeState(CALIBRATION);

            vTaskSuspend(NULL); // double suspend check
            btTx = "Calibration end";
          }
          else
          {
            btTx = "Couldn't start calibration";
          }

          // save to flash
        }
        else if (command == "WCS;")
        {
          btUI.saveToFlash();

          btTx = "Saved to flash";
          btUI.switchCalibrationFactorsFlag();
          // show current setings
        }
        else if (command == "SCS;")
        {
          btTx = btUI.timersDescription();

                //enable/disable data frame to user
                }else if(command == "SDF;"){
                    btTx = "Data frame "; //enable / disable
                    //set flag btTxFlag

                //start static fire task
                }else if(command == "SFS;"){
                    //Igniter continuity check
                    if(analogRead(CONTINUITY) > 512){
                        if(btUI.checkTimers()){//check timers
                            askTime = xTaskGetTickCount() * portTICK_PERIOD_MS; //start timer
                            btTx = btUI.timersDescription(); //show timers
                        
                            btTx += "\n\nDo you want to start test with this settings? Write MH;SFY;"; //ask
                        }else{
                            btTx = "Invalid valve setings";
                        }
                    }else{
                        askTime = 0;
                        btTx = "Lack of igniter continuity! :C";
                    }
                    
                //
                }else if(command == "SFY;"){
                    if(askTime == 0){
                        btTx = "Unknown command";
                    }else if((xTaskGetTickCount() * portTICK_PERIOD_MS) - askTime < askTimeOut){
                        btUI.saveToFlash();
                        btTx = "create static fire task";
                        askTime = 0;
                        sm.changeState(COUNTDOWN);
                    }else{
                        btTx = "Static fire ask time out!";
                        askTime = 0;
                    }
                
                //turn off esp 
                }else if(command == "RST;"){
                    btTx = "Esp is going to sleep";
                }
        else if (command == "BTF;")
        {
          if (btUI.switchDataFlag())
            btTx = "Switched BT Data Flag";
          else
          {
            btTx = "ERROR: cannot switch BT Data Flag";
          }
        }
        else
        {
          btTx = "Unknown command";
        }

        if (command != "SFS;" && askTime != 0)
        {
          askTime = 0;
        }

        xQueueSend(sm.btTxQueue, (void *)&btTx, 10);
      }
      else
      {
        btTx = "Unknown command or not IDLE state";
        xQueueSend(sm.btTxQueue, (void *)&btTx, 10);
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

//***********************************//
//             APP_CPU               //
//***********************************//

void stateTask(void *arg)
{
  String stateMsg;
  while (1)
  {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE)
    {
      // critical section begin
      portENTER_CRITICAL(&sm.spinlock);
      switch (sm.state)
      {
      case DISCONNECTED:
        vTaskSuspend(sm.uiTask);
        // vTaskSuspend(sm.dataTask);

        // disable for Seba i Krzysiek
        // abort after disconnect
        // sm.timer.setDefault();
        /*
        if(sm.staticFireTask != NULL){
            vTaskDelete(sm.staticFireTask);
            sm.staticFireTask = NULL;
        }*/

        break;

      case IDLE:
        // resume suspended tasks
        sm.timer.setDefault();
        vTaskResume(sm.uiTask);
        vTaskResume(sm.dataTask);

        if (sm.calibrationTask != NULL)
        {
          if (eTaskGetState(sm.calibrationTask) != eDeleted)
          {
            vTaskDelete(sm.calibrationTask);
          }
          sm.calibrationTask = NULL;
        }

        stateMsg = "State: IDLE";
        break;

      case CALIBRATION:
        vTaskDelay(10 / portTICK_PERIOD_MS);
        vTaskSuspend(sm.uiTask);   // calibration task will handle ui
        vTaskSuspend(sm.dataTask); // calibration task need only data from load cell

        xTaskCreatePinnedToCore(calibrationTask, "calibration task", 10000, NULL, 2, &sm.calibrationTask, APP_CPU_NUM);

        if (sm.calibrationTask == NULL)
        {
          stateMsg = "Beep boop error, calibrationTask not created";
          vTaskResume(sm.uiTask);
          vTaskResume(sm.dataTask);
          sm.changeState(IDLE);
        }
        else
        {
          stateMsg = "State: CALIBRATION";
        }

        break;

      case COUNTDOWN:
        xTaskCreatePinnedToCore(staticFireTask, "static fire task", 10000, NULL, 3, &sm.staticFireTask, APP_CPU_NUM);

        if (sm.staticFireTask == NULL)
        {
          stateMsg = "Beep boop error, staticFireTask not created";
          sm.changeState(IDLE);
        }
        else
        {
          stateMsg = "State: COUNTDOWN";
        }

        break;
      case STATIC_FIRE:
        stateMsg = "State: STATIC_FIRE";
        break;
      case ABORT:
        if (sm.staticFireTask != NULL)
        {
          vTaskDelete(sm.staticFireTask);
          sm.staticFireTask = NULL;
        }
        // close valve or sth
        digitalWrite(IGNITER, LOW);
        sm.timer.setDefault();
        stateMsg = "State: ABORT";
        break;
      }

      // critical section end
      portEXIT_CRITICAL(&sm.spinlock);

      // state info for user
      if (sm.state != DISCONNECTED)
      {
        xQueueSend(sm.btTxQueue, (void *)&stateMsg, 0);
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void dataTask(void *arg)
{
  String dataFrame;
  // float revDividerVal = (10000.0 + 47000.0)/10000.0;
  HX711_ADC mainLoadCell(LC1_DT, LC1_CLK);
  uint16_t stabilizingTime = 2000;
  bool _tare = true;
  uint8_t iter = 0;

  Trafag8252 pressureSens(PRESS_SENS, btUI.getPressureSensCalibrationFactor());

  mainLoadCell.begin();

  mainLoadCell.start(stabilizingTime, _tare);

  while (mainLoadCell.getTareTimeoutFlag())
  {
    dataFrame = "Main Load Cell Disconnected!";
    xQueueSend(sm.btTxQueue, (void *)&dataFrame, 10);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  mainLoadCell.setCalFactor(-620.0);
  mainLoadCell.setSamplesInUse(1);

  // data = "TIME; THRUST; OXIDANT_WEIGHT; PRESSURE; TEMP_1; TEMP_2; VALVE_1 STATE; VALVE_2 STATE; BATTERY;";
  // ^for full hardware
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // dataFrame = "TIME; THRUST; PRESSURE; VALVE_1 STATE; VALVE_2 STATE;";
  // xQueueSend(sm.sdQueue, (void *)&dataFrame, 10);

  while (1)
  {
    // czas testu możesz dostać z sm.timer.getTime();
    dataFrame = "";

    dataFrame += String(sm.timer.getTime()) + "; ";

    if (btUI.checkCalibrationFactorsFlag())
    {
      mainLoadCell.setCalFactor(-620.0);
      btUI.switchCalibrationFactorsFlag();
    }

    if (mainLoadCell.update())
    {
      dataFrame += String(mainLoadCell.getData()) + "; ";

      dataFrame += String(pressureSens.getPressure()) + "; ";

      /*
      xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

      //dataFrame = termoparaRead;

      xSemaphoerGive(sm.spiMutex, portMAX_DELAY);
      */

      dataFrame += firstValve.getPosition() + "; ";
      dataFrame += secondValve.getPosition() + "; ";
      
      if(analogRead(CONTINUITY) > 512)
      {
        dataFrame += "Jest ciaglosc; ";
      }
      else{
        dataFrame += "Nie ma ciaglosci";
      }
      // dataFrame += checkBattery(BATT_CHECK, revDividerVal);
      dataFrame += "\n";

      if (sm.timer.isEnable())
      { // timer is enable only in COUNTDOWN AND STATIC_FIRE STATE
        xQueueSend(sm.sdQueue, (void *)&dataFrame, 10);
      }

      if (btUI.checkDataFlag() && iter == 10)
      {
        // Serial.println(dataFrame);
        xQueueSend(sm.btTxQueue, (void *)&dataFrame, 10);
        iter = 0;
      }
      iter++;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void sdTask(void *arg)
{
  SDCard sd(myspi, SD_CS);
  String header = "TIME; THRUST; PRESSURE; VALVE_1 STATE; VALVE_2 STATE;\n";
  String data = "";
  uint16_t i = 0;

  vTaskDelay(100 / portTICK_RATE_MS);

  xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

  while (!sd.init())
  {
    data = "Sd init error!";
    xQueueSend(sm.btTxQueue, (void *)&data, 10);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  while (sd.fileExists("/data_test_" + String(i) + ".txt"))
  {
    i++;
  }
  String logPath = "/logs_test_" + String(i) + ".txt";
  String dataPath = "/data_test_" + String(i) + ".txt";

  sd.write(dataPath, header);
  xSemaphoreGive(sm.spiMutex);

  while (1)
  {
    if (xQueueReceive(sm.sdQueue, (void *)&data, portMAX_DELAY) == pdTRUE)
    {
      xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

      if (data.startsWith("LOG: "))
      { // write logs
        if (!sd.write(logPath, data))
        {
          // error handling
        }
      }
      else
      { // write data
        if (!sd.write(dataPath, data))
        {
          // error handling
        }
      }

      xSemaphoreGive(sm.spiMutex);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void staticFireTask(void *arg)
{
  uint16_t countDownTime = btUI.getCountDownTime();
  uint32_t firstValveOpenTime = btUI.getValveOpenTimer(FIRST_VALVE);
  uint32_t firstValveCloseTime = btUI.getValveCloseTimer(FIRST_VALVE);
  //uint8_t firstValveEnable = btUI.getValveState(FIRST_VALVE);
  //uint32_t secondValveOpenTime = btUI.getValveOpenTimer(SECOND_VALVE);
  //uint32_t secondValveCloseTime = btUI.getValveCloseTimer(SECOND_VALVE);
  //uint8_t secondValveEnable = btUI.getValveState(SECOND_VALVE);
  //uint32_t valveOpenTime;
  uint64_t testStartTime;
  uint64_t testStopTime;
  bool openFlag = false;
  bool closeFlag = false;
  bool igniterState = LOW;
  uint8_t bipTimes = 1;
  String msg;

  if(firstValveCloseTime == 0){
    closeFlag = true;
  }

  servo.attach(SERVO_PIN, 500, 2400);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // set timers
  testStartTime = millis() + countDownTime; // T0
  testStopTime = firstValveCloseTime + 30000 + testStartTime;
  sm.timer.setTimer(testStartTime);

  // countdown
  while ((testStartTime > millis()))
  {
    int timeInSec = (testStartTime - millis()) / 1000;

    if ((timeInSec > 10) && (timeInSec % 5 == 0))
    {

      msg = String(timeInSec);
      xQueueSend(sm.btTxQueue, (void *)&msg, 0);
    }
    else if (timeInSec <= 10)
    {
      bipTimes += 1;
      msg = String(timeInSec);
      xQueueSend(sm.btTxQueue, (void *)&msg, 0);
    }

    beepBoop(500, bipTimes); //delay include
    //vTaskDelay(1000  / portTICK_PERIOD_MS);
  }

  if (sm.state == ABORT)
  {
    vTaskDelete(NULL);
  }

  if (analogRead(CONTINUITY) < 512)
  {
    msg = "Brak ciaglosci zapalnika";
    xQueueSend(sm.btTxQueue, (void *)&msg, 0);
    sm.changeState(ABORT);
    vTaskDelete(NULL);
  }

  // ignition
  sm.changeState(STATIC_FIRE);
  igniterState = HIGH;
  digitalWrite(IGNITER, igniterState);
  msg = "Static fire fruuuuu!!!1!";
  xQueueSend(sm.btTxQueue, (void*)&msg, 0);

  // test start time is T0.00
  // valve control
  while (testStopTime > millis())
  {
    // open valve
    if (openFlag == false)
    {
      if ((millis() - testStartTime) > (firstValveOpenTime))
      {
        servo.write(SERVO_OPEN_POSITION);
        openFlag = true;
        msg = "Valve open";
        xQueueSend(sm.btTxQueue, (void*)&msg, 0);
      }
    }

    // close valve
    if (closeFlag == false)
    { 
      if(firstValveCloseTime == 0){
        closeFlag = true;
      }
      else if ((millis() - testStartTime) > (firstValveCloseTime))
      {
        servo.write(SERVO_CLOSE_POSITION);
        closeFlag = true;
        msg = "Valve close";
        xQueueSend(sm.btTxQueue, (void*)&msg, 0);
      }
    }

    // igniter
    if (((millis() - testStartTime) > 1000) && (igniterState == HIGH))
    {
      igniterState = LOW;
      digitalWrite(IGNITER, igniterState);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  sm.timer.setDefault();
  sm.changeState(IDLE);
  sm.staticFireTask = NULL;

  vTaskDelete(NULL);
}

void calibrationTask(void *arg)
{
  /*
  if (eTaskGetState(sm.uiTask) == eSuspended)
  {
      btMsg = "Suspended UI";
  }
  else
  {
      btMsg = "ERROR: Can't suspend UI";
  }
  xQueueSend(sm.btTxQueue, (void *)&btMsg, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  */
  String btMsg;
  String prefix = "MH;";
  String command;
  float measuredVal = 0.0;
  float mass = 0.0;
  float pressure = 0.0;
  uint8_t n = 5;
  float massVsMeasured[2][n];
  uint8_t i = 0;

  bool newDataReady = false;
  bool ifTared = false;
  uint16_t stabilizingTime = 2000;
  bool _tare = true;
  uint8_t dt = LC1_DT, clk = LC1_CLK;
  uint8_t setLoadCell = FIRST_LOAD_CELL;
  if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, portMAX_DELAY) == pdTRUE)
  {
    if (btMsg == "LC1;" || btMsg == "LC2;")
    {
      if (btMsg == "LC2;") // default is LC1
      {
        dt = LC2_DT;
        clk = LC2_CLK;
        setLoadCell = SECOND_LOAD_CELL;
      }
      HX711_ADC LoadCell(dt, clk);
      LoadCell.begin();
      LoadCell.start(stabilizingTime, _tare);

      while (LoadCell.getTareTimeoutFlag())
      {
        btMsg = "Main Load Cell Disconnected!";
        xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }

      LoadCell.setCalFactor(1.0);
      LoadCell.setSamplesInUse(1);

      while (i < n)
      {
        Serial.println(xPortGetFreeHeapSize());
        btMsg = "Type MH;TAR; to tare, then MH;MAS;zzzzz (known weight)";
        xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, portMAX_DELAY) == pdTRUE)
        {
          if (checkCommand(btMsg, prefix, ';', 2))
          {
            btMsg.remove(0, prefix.length()); // remove MH; prefix
            command = btMsg.substring(0, 4);  // get command

            if (command == "TAR;")
            {
              LoadCell.tareNoDelay();
              btMsg = "Load Cell tared. Place an object on Load Cell and type MH;MAS;zzzzz (known weight)";
              ifTared = true;
            }
            else if (command == "MAS;" && ifTared)
            {
              mass = btMsg.substring(4).toFloat();
              if (LoadCell.update())
                newDataReady = true;
              if (newDataReady)
              {
                measuredVal = LoadCell.getData();
                if (measuredVal != 0.0)
                {
                  btMsg = "Measured value is " + String(measuredVal);
                  massVsMeasured[0][i] = mass;
                  massVsMeasured[1][i] = measuredVal;
                  i++;
                }
                else
                  btMsg = "No value measured :(";

                newDataReady = false;
                ifTared = false;
              }
              else
              {
                btMsg = "Lipa";
              }
            }
            else if (command == "MAS;" && !ifTared)
            {
              btMsg = "Tare first!";
            }
            else
            {
              btMsg = "Unknown command";
            }
            xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
          }
        }
      }
      float xsum = 0, x2sum = 0, ysum = 0, xysum = 0, a, b; // variables for sums/sigma of xi,yi,xi^2,xiyi etc
      for (i = 0; i < n; i++)
      {
        xsum = xsum + massVsMeasured[0][i];                          // calculate sigma(xi)
        ysum = ysum + massVsMeasured[1][i];                          // calculate sigma(yi)
        x2sum = x2sum + massVsMeasured[0][i] * massVsMeasured[0][i]; // calculate sigma(x^2i)
        xysum = xysum + massVsMeasured[0][i] * massVsMeasured[1][i]; // calculate sigma(xi*yi)
      }
      a = (n * xysum - xsum * ysum) / (n * x2sum - xsum * xsum);     // calculate slope
      b = (x2sum * ysum - xsum * xysum) / (x2sum * n - xsum * xsum); // calculate intercept
      btMsg = "Calibration factor has been calculated and equals " + String(a) + " constant term equals " + String(b) + "\nSave to flash? (Y/N)";
      xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
      if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, portMAX_DELAY) == pdTRUE)
      {
        if (btMsg.equalsIgnoreCase("Y"))
        {
          btUI.setCalibrationFactor((uint16_t)a, setLoadCell);
          btUI.switchCalibrationFactorsFlag();
          btUI.saveToFlash();
          btMsg = "Saved value to flash";
        }
        else
        {
          btMsg = "Did not save to flash";
        }
        xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
      }
    }
    else if (btMsg == "PSC;")
    {

      btMsg = "Type MH;";
    }
  }

  if (sm.state != DISCONNECTED)
  {
    sm.changeState(IDLE);
  }
  sm.calibrationTask = NULL;
  vTaskDelete(NULL);
}