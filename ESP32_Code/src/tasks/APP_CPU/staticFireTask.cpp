#include "../../../include/tasks/loopTasks.h"

#define TEST_TIME 1800000


void staticFireTask(void *arg)
{
  uint16_t countDownTime = btUI.getCountDownTime();
  uint64_t dataLogingTime = btUI.getTestTime();
  uint32_t firstValveOpenTime = btUI.getValveOpenTimer(FIRST_VALVE);
  uint32_t firstValveCloseTime = btUI.getValveCloseTimer(FIRST_VALVE);
  uint8_t firstValveEnable = btUI.getValveState(FIRST_VALVE);
  uint32_t secondValveOpenTime = btUI.getValveOpenTimer(SECOND_VALVE);
  uint32_t secondValveCloseTime = btUI.getValveCloseTimer(SECOND_VALVE);
  uint8_t secondValveEnable = btUI.getValveState(SECOND_VALVE);
  uint32_t valveOpenTime;
  uint32_t testStartTime;
  uint32_t testStopTime;
  TaskHandle_t firstValveTask = NULL;
  TaskHandle_t secondValveTask = NULL;
  bool igniterState = LOW;
  uint8_t bipTimes = 1;
  char msg[100];
  //float maxThrust = 0.0;
  vTaskDelay(100 / portTICK_PERIOD_MS);

  // set timers
  testStartTime = millis() + countDownTime; // T0
  testStopTime = (firstValveCloseTime > secondValveCloseTime ? firstValveCloseTime : secondValveCloseTime) + dataLogingTime + testStartTime;
  sm.timer.setTimer(testStartTime);

  // countdown
  while ((testStartTime > millis()))
  {
    int timeInSec = (testStartTime - millis()) / 1000;

    if ((timeInSec > 10) && (timeInSec % 5 == 0))
    {
      sprintf(msg, "T: -%d", timeInSec);
      xQueueSend(sm.btTxQueue, &msg, 0);
    }
    else if (timeInSec <= 10)
    {
      bipTimes += 1;
      sprintf(msg, "%d", timeInSec);
      xQueueSend(sm.btTxQueue, &msg, 0);
    }

    beepBoop(500, bipTimes); // delay include
    // vTaskDelay(1000  / portTICK_PERIOD_MS);
  }

  if (sm.state == ABORT)
  {
    sprintf(msg, "ABORT");
    xQueueSend(sm.btTxQueue, &msg, 0);
    sm.timer.setDefault();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    sm.staticFireTask = NULL;
    vTaskDelete(NULL);
  }

  xSemaphoreTake(sm.analogMutex, portMAX_DELAY);
  if ((analogRead(CONTINUITY) < 512 && igniterState == LOW) && !btUI.checkCTFlag())
  {
    sprintf(msg, "Brak ciaglosci zapalnika - ABORT");
    xQueueSend(sm.btTxQueue, &msg, 0);
    sm.changeState(ABORT);
    sm.timer.setDefault();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    sm.staticFireTask = NULL;
    vTaskDelete(NULL);
  }
  xSemaphoreGive(sm.analogMutex);

  // ignition
  sm.changeState(STATIC_FIRE);
  //xQueueSend(sm.btTxQueue, &msg, 0);
  igniterState = HIGH;
  digitalWrite(IGNITER, igniterState);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  sprintf(msg, "Rozpoczecie static fire.");
  xQueueSend(sm.btTxQueue, &msg, 0);

  // test start time is T0.00
  // valve control
  while (testStopTime > millis())
  {
    // first valve
    if (firstValveEnable && (firstValveTask == NULL))
    {
      if ((millis() - testStartTime) > (firstValveOpenTime))
      {
        // time open
        if (firstValveCloseTime != 0)
        {
          valveOpenTime = firstValveCloseTime - firstValveOpenTime;
          xTaskCreatePinnedToCore(timeOpenFirstValve, "Valve 1", 1500, (void *)&valveOpenTime, 2, &firstValveTask, APP_CPU_NUM);

          sprintf(msg, "First valve open for: %d", valveOpenTime);
          xQueueSend(sm.btTxQueue, &msg, 0);
          sprintf(msg, "LOG: First valve open for: %d", valveOpenTime);
          xQueueSend(sm.sdQueue, &msg, 0);
          // open
        }
        else
        {
          xTaskCreatePinnedToCore(openFirstValve, "Valve 1 open", 1500, NULL, 2, &firstValveTask, APP_CPU_NUM);

          sprintf(msg, "First valve open");
          xQueueSend(sm.btTxQueue, &msg, 0);
          sprintf(msg, "LOG: First valve open for: %d", valveOpenTime);
          xQueueSend(sm.sdQueue, &msg, 0);
        }
      }
    }

    // second valve
    if (secondValveEnable && (secondValveTask == NULL))
    {
      if ((millis() - testStartTime) > (secondValveOpenTime))
      {
        // time open
        if (secondValveCloseTime != 0)
        {
          valveOpenTime = secondValveCloseTime - secondValveOpenTime;
          xTaskCreatePinnedToCore(timeOpenSecondValve, "Valve 2", 1500, (void *)&valveOpenTime, 2, &secondValveTask, APP_CPU_NUM);

          sprintf(msg, "Second valve open for: %d", valveOpenTime);
          xQueueSend(sm.btTxQueue, &msg, 0);
          sprintf(msg, "LOG: Second valve open for: %d", valveOpenTime);
          xQueueSend(sm.sdQueue, &msg, 0);
          // open
        }
        else
        {
          xTaskCreatePinnedToCore(openSecondValve, "Valve 2 open", 1500, NULL, 2, &secondValveTask, APP_CPU_NUM);

          sprintf(msg, "Second valve open");
          xQueueSend(sm.btTxQueue, &msg, 0);
          sprintf(msg, "LOG: Second valve open for: %d", valveOpenTime);
          xQueueSend(sm.sdQueue, &msg, 0);
        }
        //if()
      }
    }

    // igniter
    if (((millis() - testStartTime) > 3000) && (igniterState == HIGH)) //Turn off the igniter
    {
      igniterState = LOW;
      digitalWrite(IGNITER, igniterState);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  sprintf(msg, "End of static fire - end of data logging.");
  xQueueSend(sm.btTxQueue, &msg, 0);
  sm.timer.setDefault();
  sm.changeState(IDLE);
  sm.staticFireTask = NULL;

  vTaskDelete(NULL);
}
