#include "../../../include/tasks/loopTasks.h"

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
  uint8_t n = 3;
  float massVsMeasured[2][n];
  uint8_t i = 0, j = 0;

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
        // Serial.println(xPortGetFreeHeapSize());
        btMsg = "Measurement " + String(i + 1) + ". Type MH;TAR; to tare, the known weight";
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
              btMsg = "Load Cell tared. Place an object on Load Cell its own weight (float)";
              ifTared = true;
            }
            else
            {
              btMsg = "ERROR: Type MH;TAR; to tare the load cell";
            }
            xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
          }
          if (xQueueReceive(sm.btRxQueue, (void *)&btMsg, portMAX_DELAY) == pdTRUE)
          {
            if (btMsg.toFloat() && ifTared)
            {
              mass = btMsg.toFloat();
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
              xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
            }
            else
            {
              btMsg = "Not a valid float";
              xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
            }
          }
        }
      }

      for (j = 0; j < n; j++)
      {
        btMsg += String(massVsMeasured[0][j]) + " " + String(massVsMeasured[1][j]) + " ";
      }

      xQueueSend(sm.btTxQueue, (void *)&btMsg, 10);
      float xsum = 0, x2sum = 0, ysum = 0, xysum = 0, a, b; // variables for sums/sigma of xi,yi,xi^2,xiyi etc
      for (j = 0; j < n; j++)
      {
        xsum = xsum + massVsMeasured[0][j];                          // calculate sigma(xi)
        ysum = ysum + massVsMeasured[1][j];                          // calculate sigma(yi)
        x2sum = x2sum + massVsMeasured[0][j] * massVsMeasured[0][j]; // calculate sigma(x^2i)
        xysum = xysum + massVsMeasured[0][j] * massVsMeasured[1][j]; // calculate sigma(xi*yi)
      }
      a = (n * xysum - xsum * ysum) / (n * x2sum - xsum * xsum);     // calculate slope
      b = (x2sum * ysum - xsum * xysum) / (x2sum * n - xsum * xsum); // calculate intercept
      btMsg = "Calibration factor has been calculated and equals " + String(a) + " constant term equals " + String(b) + ". Save to flash? (Y/N)";
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
}