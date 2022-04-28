#include "../../../include/tasks/loopTasks.h"

void staticFireTask(void *arg)
{
  uint16_t countDownTime = btUI.getCountDownTime();
  uint32_t firstValveOpenTime = btUI.getValveOpenTimer(FIRST_VALVE);
  uint32_t firstValveCloseTime = btUI.getValveCloseTimer(FIRST_VALVE);
  uint8_t firstValveEnable = btUI.getValveState(FIRST_VALVE);
  uint32_t secondValveOpenTime = btUI.getValveOpenTimer(SECOND_VALVE);
  uint32_t secondValveCloseTime = btUI.getValveCloseTimer(SECOND_VALVE);
  uint8_t secondValveEnable = btUI.getValveState(SECOND_VALVE);
  uint32_t valveOpenTime;
  uint64_t testStartTime;
  uint64_t testStopTime;
  TaskHandle_t firstValveTask = NULL;
  TaskHandle_t secondValveTask = NULL;
  bool igniterState = LOW;
  uint8_t bipTimes = 1;
  String msg;

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // set timers
  testStartTime = millis() + countDownTime; // T0
  testStopTime = (firstValveCloseTime > secondValveCloseTime ? firstValveCloseTime : secondValveCloseTime) + 30000 + testStartTime;
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

    beepBoop(500, bipTimes); // delay include
    // vTaskDelay(1000  / portTICK_PERIOD_MS);
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
  xQueueSend(sm.btTxQueue, (void *)&msg, 0);

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

          msg = "First valve open for: " + String(valveOpenTime);
          xQueueSend(sm.btTxQueue, (void *)&msg, 0);
          // open
        }
        else
        {
          xTaskCreatePinnedToCore(openFirstValve, "Valve 1 open", 1500, NULL, 2, &firstValveTask, APP_CPU_NUM);

          msg = "First valve open";
          xQueueSend(sm.btTxQueue, (void *)&msg, 0);
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

          msg = "Second valve open for: " + String(valveOpenTime);
          xQueueSend(sm.btTxQueue, (void *)&msg, 0);
          // open
        }
        else
        {
          xTaskCreatePinnedToCore(openSecondValve, "Valve 2 open", 1500, NULL, 2, &secondValveTask, APP_CPU_NUM);

          msg = "Second valve open";
          xQueueSend(sm.btTxQueue, (void *)&msg, 0);
        }
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
