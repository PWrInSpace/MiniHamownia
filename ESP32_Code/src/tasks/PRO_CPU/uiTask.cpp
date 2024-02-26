#include "../../../include/tasks/loopTasks.h"

void uiTask(void *arg)
{
  String btMsg;
  String prefix = "MH;"; // prefix
  String command;
  uint32_t time;
  char btTx[100];
  TickType_t askTime = 0;
  TickType_t askTimeOut = 30000;

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
        if (command == "MO1;" || command == "MO2;")
        {
          if (command[2] == '1')
          {
            xTaskCreatePinnedToCore(openFirstValve, "First Val open", 2048, NULL, 2, NULL, APP_CPU_NUM);
            sprintf(btTx, "First valve open");
          }
          else if (command[2] == '2')
          {
            xTaskCreatePinnedToCore(openSecondValve, "second Val open", 2048, NULL, 2, NULL, APP_CPU_NUM);
            sprintf(btTx, "Second valve open");
          }
          else
          {
            sprintf(btTx, "Unknown valve number");
          }
        }
        else if (command == "MC1;" || command == "MC2;")
        {
          if (command[2] == '1')
          {
            xTaskCreatePinnedToCore(closeFirstValve, "First Val close", 2048, NULL, 2, NULL, APP_CPU_NUM);
            sprintf(btTx, "First valve closed");
          }
          else if (command[2] == '2')
          {
            xTaskCreatePinnedToCore(closeSecondValve, "second Val close", 2048, NULL, 2, NULL, APP_CPU_NUM);
            sprintf(btTx, "Second valve closed");
          }
          else
          {
            sprintf(btTx, "Unknown valve number");
          }
        }
        else if (command == "TO1;" || command == "TO2;")
        {
          if (command[2] == '1')
          {
            xTaskCreatePinnedToCore(timeOpenFirstValve, "First val time open", 2048, (void *)&time, 2, NULL, APP_CPU_NUM); // i think it won't work xDD
            sprintf(btTx, "First valve open for %d", time);
          }
          else if (command[2] == '2')
          {
            xTaskCreatePinnedToCore(timeOpenSecondValve, "Second val time open", 2048, (void *)&time, 2, NULL, APP_CPU_NUM);
            sprintf(btTx, "First valve open for %d", time);
          }
          else
          {
            sprintf(btTx, "Unknown valve number");
          }
        }
        else if (command == "VO1;" || command == "VO2;")
        {
          if (btUI.setValveOpenTimer(time, command[2] - 48))
          {
            sprintf(btTx, "Valve %s New open time: %d", command[2] - 48, btUI.getValveOpenTimer(command[2] - 48));
          }
          else
          {
            sprintf(btTx, "Failed to save");
          }

          // valve close timer
        }
        else if (command == "VC1;" || command == "VC2;")
        {
          if (btUI.setValveCloseTimer(time, command[2] - 48))
          {
            sprintf(btTx, "Valve %s New close time: %d", command[2] - 48, btUI.getValveCloseTimer(command[2] - 48));
          }
          else
          {
            sprintf(btTx, "Failed to save");
          }

          // valve enable
        }
        else if (command == "VE1;" || command == "VE2;")
        {
          if (btUI.setValveState(time, command[2] - 48))
          {
            sprintf(btTx, "Valve %s %s", String(command[2] - 48), (btUI.getValveState(command[2] - 48) > 0 ? "enable" : "disable"));
          }
          else
          {
            sprintf(btTx, "Failed to save");
          }

          // count down timer
        }
        else if (command == "CDT;")
        {
          if (btUI.setCountDownTime(time))
          {
            sprintf(btTx, "New countdown time: %d", btUI.getCountDownTime());
          }
          else
          {
            sprintf(btTx, "Failed to save");
          }
        }
        else if (command == "DLT;")
        {
          if(btUI.setTestTime(time))
          {
            sprintf(btTx, "New data loging time: %d", btUI.getTestTime());
          }
          else
          {
            sprintf(btTx, "Failed to save: %d", time);
          }
        }
        else if (command == "JP1;")
        {
          btUI.setCalibrationFactor(time, FIRST_LOAD_CELL); // value not time :D
          sprintf(btTx, "New calibration factor: %d", btUI.getCalibrationFactor(FIRST_LOAD_CELL));
        }
        else if (command == "JP2;")
        {
          btUI.setCalibrationFactor(time, SECOND_LOAD_CELL); // value not time :D
          sprintf(btTx, "New calibration factor: %d", btUI.getCalibrationFactor(SECOND_LOAD_CELL));
        }
        else if (command == "JP3;")
        {
          btUI.setPressureSensCalibrationFactor(time); // value not time :)
          sprintf(btTx, "New pressure sens calibration factor: %d", btUI.getPressureSensCalibrationFactor());
        }
        else if (command == "LC1;" || command == "LC2;" || command == "PSC;")
        {
          if (xQueueSend(sm.btRxQueue, &command, 0) == pdTRUE)
          {
            sm.changeState(CALIBRATION);

            vTaskSuspend(NULL); // double suspend check
            sprintf(btTx, "Calibration end");
          }
          else
          {
            sprintf(btTx, "Couldn't start calibration");
          }

          // save to flash
        }
        else if (command == "WCS;")
        {
          btUI.saveToFlash();

          sprintf(btTx, "Saved to flash");
          btUI.switchCalibrationFactorsFlag();
          // show current setings
        }
        else if (command == "SCS;")
        {
          btUI.println(btUI.timersDescription());

          // enable/disable data frame to user
        }
        else if (command == "SDF;")
        {
          sprintf(btTx, "Data frame "); // enable / disable
          // set flag btTxFlag

          // start static fire task
        }
        else if (command == "SFS;")
        {
          // Igniter continuity check
          xSemaphoreTake(sm.analogMutex, portMAX_DELAY);
          if (analogRead(CONTINUITY) > 512 || btUI.checkCTFlag())
          {
            if (btUI.checkTimers())
            {                                                     // check timers
              askTime = xTaskGetTickCount() * portTICK_PERIOD_MS; // start timer
              btUI.println(btUI.timersDescription());
              sprintf(btTx, "\n\nDo you want to start test with this settings? Write MH;SFY;");                    // show timers
            }
            else
            {
              sprintf(btTx, "Invalid valve setings");
            }
          }
          else
          {
            askTime = 0;
            sprintf(btTx, "Lack of igniter continuity! :C");
          }
          xSemaphoreGive(sm.analogMutex);
          //
        }
        else if (command == "SFY;")
        {
          if (askTime == 0)
          {
            sprintf(btTx, "Unknown command");
          }
          else if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - askTime < askTimeOut)
          {
            btUI.saveToFlash();
            sprintf(btTx, "create static fire task");
            askTime = 0;
            sm.changeState(COUNTDOWN);
          }
          else
          {
            sprintf(btTx, "Static fire ask time out!");
            askTime = 0;
          }
        }
        else if (command == "RST;") // turn off esp
        {
          sprintf(btTx, "Esp is going to sleep");
          ESP.restart();
        }
        else if (command == "BTF;")
        {
          if (btUI.switchBtFlag())
            sprintf(btTx, "Switched BT Data Flag");
          else
          {
            sprintf(btTx, "ERROR: cannot switch BT Data Flag");
          }
        }
        else if (command == "CTO;") // Overrides continuity check - use with externaly controlled hybrid / liquid motors
        {
          if (btUI.switchCtFlag())
            sprintf(btTx, "Switched Continuity Override Flag");
          else
          {
            sprintf(btTx, "ERROR: cannot switch Continuity Override Flag");
          }
        }
        else if(command == "TA1;")
        {
          btUI.switchTareFlag(_mainLoadCell);
          sprintf(btTx, "Taring main load cell...");
        }
        else
        {
          sprintf(btTx, "Unknown command");
        }

        if (command != "SFS;" && askTime != 0)
        {
          askTime = 0;
        }
        xQueueSend(sm.btTxQueue, &btTx, 10);
      }
      else if(checkCommand(btMsg, prefix, ';', 2) && (sm.state == STATIC_FIRE))
      {
        btMsg.remove(0, prefix.length());  // remove MH; prefix
        command = btMsg.substring(0, 4); 
        if(command == "STP;")
        {
          if(btUI.setTestTime((xTaskGetTickCount() * portTICK_PERIOD_MS) + 1000))
          {
            sprintf(btTx, "Test will end at: %d", btUI.getTestTime());
          }
          else
          {
            sprintf(btTx, "Failed to stop the test.");
          }
        }
        else
        {
          sprintf(btTx, "Unknown command or not IDLE state");
        }
        xQueueSend(sm.btTxQueue, &btTx, 10);
      }
      else
      {
        sprintf(btTx, "Unknown command or not IDLE state");
        xQueueSend(sm.btTxQueue, &btTx, 10);
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}