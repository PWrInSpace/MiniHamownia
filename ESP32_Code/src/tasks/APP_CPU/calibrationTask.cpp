#include "../../../include/tasks/loopTasks.h"

constexpr uint8_t CAL_SAMPLES = 10;
constexpr float DEFAULT_CAL = 100.0;

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
  float measuredVal[CAL_SAMPLES];
  float mass = 0.0, massSum = 0.0;
  float pressure = 0.0;
  uint8_t n = 3;
  uint8_t i = 0, j = 0;

  bool newDataReady = false;
  bool ifValid = false;
  uint16_t stabilizingTime = 2000;
  bool _tare = true;
  uint8_t dt = LC1_DT, clk = LC1_CLK;
  uint8_t setLoadCell = FIRST_LOAD_CELL;
  float newCalFactor;

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

      LoadCell.setCalFactor(DEFAULT_CAL);
      LoadCell.setSamplesInUse(10);

      btMsg = "Type in the weight in N (float)";
      xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
      do
      {
        if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, portMAX_DELAY) == pdTRUE)
        {
          if (btMsg.toFloat())
          {
            mass = btMsg.toFloat();
            ifValid = 1;
          }
          else
          {
            btMsg = "Not a valid float, try again";
            xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
          }
        }
      } while (!ifValid);

      for (i = 0; i < CAL_SAMPLES; ++i)
      {
        if (LoadCell.update() == 1)
          measuredVal[i] = LoadCell.getData();
        if (measuredVal[i] == 0)
        {
          i--;
        }
        else
        {
          btMsg = "Data no." + String(i) + " = " + String(measuredVal[i]);
          xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
          massSum += measuredVal[i];
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
      }

      newCalFactor = DEFAULT_CAL * massSum / ((float)CAL_SAMPLES) / mass;

      btMsg = "Do you want to save the new calibration value? > " + String(newCalFactor);
      xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);

      if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, portMAX_DELAY) == pdTRUE)
      {
        if (btMsg.equalsIgnoreCase("Y"))
        {
          btUI.setCalibrationFactor((uint16_t)newCalFactor, setLoadCell);
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
  }
  else if (btMsg == "PSC;")
  {
    Trafag8252 pressureSens(PRESS_SENS, 1.0);
    btMsg = "Type pressure value in Bar (float)";
    xQueueSend(sm.btTxQueue, (void *)&btMsg, 50);

    if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, portMAX_DELAY) == pdTRUE)
    {
      if (btMsg.toFloat())
      {
        pressure = btMsg.toFloat();
      }
      float newCalibrationVal = pressureSens.sensorCalibration(pressure);
      btMsg = "New calibration value is " + String(newCalibrationVal) + ". Save to flash? (Y/N)";
      xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
      if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, portMAX_DELAY) == pdTRUE)
      {
        if (btMsg.equalsIgnoreCase("Y"))
        {
          btUI.setPressureSensCalibrationFactor((uint16_t)newCalibrationVal);
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
  }

  if (sm.state != DISCONNECTED)
  {
    sm.changeState(IDLE);
  }
  sm.calibrationTask = NULL;
  vTaskDelete(NULL);
}
