#include "../../../include/tasks/loopTasks.h"

#define BATT_DIV (10000.0 + 47000.0) / 10000.0
#define PRESS1_DIV (4566.0 + 9890.0) / 4566.0
#define PRESS2_DIV (4710.0 + 9760.0) / 4710.0

void dataTask(void *arg)
{
  char dataFrame[128], infoFrame[128];
  int32_t MeasurementTime;

  // Load Cells
  HX711_ADC mainLoadCell(LC1_DT, LC1_CLK);
  uint16_t stabilizingTime = 5000;
  uint8_t iter = 0;
  // Pressure sens
  Trafag8252 pressureSens1(PRESS_SENS1, PRESS1_DIV);
  Trafag8252 pressureSens2(PRESS_SENS2, PRESS2_DIV);
  ContinuityCheck ctnChck1(CONTINUITY);
  BatteryCheck BtChck1(BATT_CHECK, BATT_DIV);
  float MeasurementPressure1;
  float MeasurementPressure2;
  float MeasurementThrust;

  // Main Load Cell
  mainLoadCell.begin();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  mainLoadCell.start(stabilizingTime);
  mainLoadCell.setCalFactor(btUI.getCalibrationFactor(1));
  mainLoadCell.setSamplesInUse(1);

  // Igniter continuity
  bool continuity = 0;

  //Battery measurement
  float MeasurementBattery;

  while (mainLoadCell.getTareTimeoutFlag())
  {
    sprintf(infoFrame, "LOG: %d Main Load Cell Disconnected!\n", MeasurementTime);
    xQueueSend(sm.btTxQueue, (void *)&infoFrame, 10);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }



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

    if (mainLoadCell.update() == 1)
    {
      MeasurementThrust = abs(mainLoadCell.getData());

      xSemaphoreTake(sm.analogMutex, portMAX_DELAY);
      MeasurementPressure1 = pressureSens1.getPressure();
      MeasurementPressure2 = pressureSens2.getPressure();
      continuity = ctnChck1.getContinuity();
      MeasurementBattery = BtChck1.getBattery();
      xSemaphoreGive(sm.analogMutex);

      sprintf(dataFrame, "%d; %4.2f; %4.2f; %4.2f; %d; %d; %d; %4.2f\n", MeasurementTime, MeasurementThrust, MeasurementPressure1, MeasurementPressure2, continuity, firstValve.getPosition(), secondValve.getPosition(), MeasurementBattery);

      if (sm.timer.isEnable())
      { // timer is enable only in COUNTDOWN AND STATIC_FIRE STATE
        xQueueSend(sm.sdQueue, &dataFrame, 10);
      }
      

      if (btUI.checkBtFlag() && iter == 80)
      {
        xQueueSend(sm.btTxQueue, &dataFrame, 10);
        iter = 0;
      }

      if(iter == 80)
      {
        iter = 0;
      }
      iter++;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}