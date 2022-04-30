#include "../../../include/tasks/loopTasks.h"

void dataTask(void *arg)
{
  String dataFrame;
  float revDividerVal = (10000.0 + 47000.0) / 10000.0;

  // Load Cells
  HX711_ADC mainLoadCell(LC1_DT, LC1_CLK);
  HX711_ADC oxidizerLoadCell(LC2_DT, LC2_CLK);
  uint16_t stabilizingTime = 2000;
  bool _tare = true;
  uint8_t iter = 0;

  // Pressure sens
  Trafag8252 pressureSens(PRESS_SENS, btUI.getPressureSensCalibrationFactor());

  // Thermocouples
  xSemaphoreTake(sm.spiMutex, portMAX_DELAY);
  MAX6675 firstThermo(&myspi, THERMO1_CS);
  MAX6675 secondThermo(&myspi, THERMO2_CS);
  xSemaphoreGive(sm.spiMutex);

  // Main Load Cell
  mainLoadCell.begin();
  mainLoadCell.start(stabilizingTime, _tare);

  // Igniter continuity
  bool continuity = 0;

  while (mainLoadCell.getTareTimeoutFlag())
  {
    dataFrame = "Main Load Cell Disconnected!";
    xQueueSend(sm.btTxQueue, (void *)&dataFrame, 10);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  mainLoadCell.setCalFactor(btUI.getCalibrationFactor(1));
  mainLoadCell.setSamplesInUse(1);

  // Oxidizer Load Cell
  oxidizerLoadCell.begin();
  oxidizerLoadCell.start(stabilizingTime, _tare);

  while (oxidizerLoadCell.getTareTimeoutFlag())
  {
    dataFrame = "oxidizer Load Cell Disconnected!";
    xQueueSend(sm.btTxQueue, (void *)&dataFrame, 10);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  oxidizerLoadCell.setCalFactor(btUI.getCalibrationFactor(1));
  oxidizerLoadCell.setSamplesInUse(1);

  // data = "TIME; THRUST; OXIDANT_WEIGHT; PRESSURE; TEMP_1; TEMP_2; VALVE_1 STATE; VALVE_2 STATE; BATTERY;";
  // ^for full hardware
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  dataFrame = "TIME; THRUST; OXIDANT_WEIGHT; CONTINUITY; PRESSURE; TEMP_1; TEMP_2; VALVE_1 STATE; VALVE_2 STATE; BATTERY;";
  xQueueSend(sm.sdQueue, (void *)&dataFrame, 10);

  while (1)
  {
    // czas testu możesz dostać z sm.timer.getTime();
    dataFrame = "";

    dataFrame += String(sm.timer.getTime()) + "; ";

    if (btUI.checkCalibrationFactorsFlag())
    {
      mainLoadCell.setCalFactor(btUI.getCalibrationFactor(1));
      oxidizerLoadCell.setCalFactor(btUI.getCalibrationFactor(1));
      btUI.switchCalibrationFactorsFlag();
    }

    if (btUI.checkTareFlag())
    {
      mainLoadCell.tareNoDelay();
      btUI.switchTareFlag();
    }

    if (mainLoadCell.update() == 1 || oxidizerLoadCell.update() == 1)
    {
      dataFrame += String(mainLoadCell.getData()) + "; ";
      dataFrame += String(oxidizerLoadCell.getData()) + "; ";
      dataFrame += String(pressureSens.getPressure()) + "; ";
      if(analogRead(CONTINUITY) > 512)
      {
        dataFrame += "Continuity; ";
      }
      else
      {
        dataFrame += "No Continuity; ";
      }
      xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

      dataFrame += String(firstThermo.readCelsius()) + "; ";
      dataFrame += String(secondThermo.readCelsius()) + "; ";

      xSemaphoreGive(sm.spiMutex);

      dataFrame += firstValve.getPosition() + "; ";
      dataFrame += secondValve.getPosition() + "; ";

      dataFrame += checkBattery(BATT_CHECK, revDividerVal);
      dataFrame += "\n";

      if (sm.timer.isEnable())
      { // timer is enable only in COUNTDOWN AND STATIC_FIRE STATE
        xQueueSend(sm.sdQueue, (void *)&dataFrame, 10);
      }
      

      if (btUI.checkDataFlag() && iter == 10)
      {
        xQueueSend(sm.btTxQueue, (void *)&dataFrame, 10);
        iter = 0;
      }

      if(!sm.timer.isEnable())
      { // BeepBoop when igniter continuity changes
        if (!continuity && analogRead(CONTINUITY) > 512)
        {
          beepBoop(300, 3);
          continuity = 1;
        }
        else if (continuity && analogRead(CONTINUITY) <= 512)
        {
          beepBoop(300, 3);
          continuity = 0;
        }
      }
      Serial.println(dataFrame);
      iter++;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}