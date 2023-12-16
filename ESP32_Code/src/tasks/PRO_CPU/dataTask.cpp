#include "../../../include/tasks/loopTasks.h"

void dataTask(void *arg)
{
  char dataFrame[100], infoFrame[100];
  int32_t MeasurementTime;
  //String dataFrame, infoFrame;
  float revDividerVal = (10000.0 + 47000.0) / 10000.0;

  // Load Cells
  HX711_ADC mainLoadCell(LC1_DT, LC1_CLK);
  uint16_t stabilizingTime = 5000;
  bool _tare = true;
  uint8_t iter = 0;

  // Pressure sens
  Trafag8252 pressureSens(PRESS_SENS);
  float MeasurementPressure;

  // Thermocouples
  //xSemaphoreTake(sm.spiMutex, portMAX_DELAY);
  //MAX6675 firstThermo(&myspi, THERMO1_CS);
  //MAX6675 secondThermo(&myspi, THERMO2_CS);
  //xSemaphoreGive(sm.spiMutex);

  // Main Load Cell
  mainLoadCell.begin();
  vTaskDelay(30 / portTICK_PERIOD_MS);
  mainLoadCell.start(stabilizingTime, _tare);
  float MeasurementThrust;

  // Igniter continuity
  bool continuity = 0;

  //Battery measurement
  float MeasurementBattery;

  //Continuity analogue reading
  u_int16_t tempAnal;

  while (mainLoadCell.getTareTimeoutFlag())
  {
    sprintf(infoFrame, "LOG: %d Main Load Cell Disconnected!\n", MeasurementTime);
    xQueueSend(sm.btTxQueue, (void *)&infoFrame, 10);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  mainLoadCell.setCalFactor(btUI.getCalibrationFactor(1));
  mainLoadCell.setSamplesInUse(1);

  // data = "TIME; THRUST; PRESSURE; TEMP_1; TEMP_2; VALVE_1 STATE; VALVE_2 STATE; BATTERY;";
  // ^for full hardware
  vTaskDelay(10 / portTICK_PERIOD_MS);

  //dataFrame = "TIME; THRUST; CONTINUITY; PRESSURE; TEMP_1; TEMP_2; VALVE_1 STATE; VALVE_2 STATE; BATTERY;";
  //xQueueSend(sm.sdQueue, (void *)&dataFrame, 10);

  //vTaskDelay(100 / portTICK_PERIOD_MS);

  while (1)
  {
    MeasurementTime = sm.timer.getTime();
    // czas testu możesz dostać z sm.timer.getTime();
    if (btUI.checkCalibrationFactorsFlag())
    {
      mainLoadCell.setCalFactor(btUI.getCalibrationFactor(1));
      btUI.switchCalibrationFactorsFlag();
    }

    if (btUI.checkTareFlag(_mainLoadCell))
    {
      mainLoadCell.tareNoDelay();
      btUI.switchTareFlag(_mainLoadCell);
      sprintf(infoFrame, "LOG: %d Tared Main Load Cell\n", MeasurementTime);
      xQueueSend(sm.btTxQueue, (void*) &infoFrame, 10);
    }

    if (mainLoadCell.update() == 1 )
    {
      MeasurementThrust = abs(mainLoadCell.getData());

      xSemaphoreTake(sm.analogMutex, portMAX_DELAY);
      MeasurementPressure = pressureSens.getPressure();
      xSemaphoreGive(sm.analogMutex);

      xSemaphoreTake(sm.analogMutex, portMAX_DELAY);
      tempAnal = analogRead(CONTINUITY);
      xSemaphoreGive(sm.analogMutex);

      //xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

      //dataFrame += String(firstThermo.readCelsius()) + "; ";
      //dataFrame += String(secondThermo.readCelsius()) + "; ";

      //xSemaphoreGive(sm.spiMutex);

      xSemaphoreTake(sm.analogMutex, portMAX_DELAY);
      MeasurementBattery = checkBattery(BATT_CHECK, revDividerVal);
      xSemaphoreGive(sm.analogMutex);

      sprintf(dataFrame, "%d; %4.2f; %4.2f; %d; %d; %d; %4.2f\n", MeasurementTime, MeasurementThrust, MeasurementPressure, (tempAnal > 512) ? 1 : 0, firstValve.getPosition(), secondValve.getPosition(), MeasurementBattery);

      if (sm.timer.isEnable())
      { // timer is enable only in COUNTDOWN AND STATIC_FIRE STATE
        xQueueSend(sm.sdQueue, &dataFrame, 10);
      }
      

      if (btUI.checkDataFlag() && iter == 80)
      {
        xQueueSend(sm.btTxQueue, &dataFrame, 10);
        iter = 0;
      }

      if(iter == 80)
      {
        iter = 0;
      }

      // if(!sm.timer.isEnable())
      // { // BeepBoop when igniter continuity changes
      //   if (!continuity && analogRead(CONTINUITY) > 512)
      //   {
      //     beepBoop(300, 3);
      //     continuity = 1;
      //   }
      //   else if (continuity && analogRead(CONTINUITY) <= 512)
      //   {
      //     beepBoop(300, 3);
      //     continuity = 0;
      //   }
      // }
      //Serial.println(dataFrame);
      iter++;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}