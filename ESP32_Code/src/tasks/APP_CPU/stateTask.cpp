#include "../../../include/tasks/loopTasks.h"


void stateTask(void *arg)
{
  char stateMsg[100];
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
        sprintf(stateMsg, "State: IDLE");
        break;

      case CALIBRATION:
        vTaskDelay(10 / portTICK_PERIOD_MS);
        vTaskSuspend(sm.uiTask);   // calibration task will handle ui
        vTaskSuspend(sm.dataTask); // calibration task need only data from load cell

        xTaskCreatePinnedToCore(calibrationTask, "calibration task", 8192, NULL, 2, &sm.calibrationTask, APP_CPU_NUM);

        if (sm.calibrationTask == NULL)
        {
          sprintf(stateMsg, "Error - calibrationTask not created");
          vTaskResume(sm.uiTask);
          vTaskResume(sm.dataTask);
          sm.changeState(IDLE);
        }
        else
        {
          sprintf(stateMsg, "State: CALIBRATION");
        }

        break;

      case COUNTDOWN:
        xTaskCreatePinnedToCore(staticFireTask, "static fire task", 8192, NULL, 3, &sm.staticFireTask, APP_CPU_NUM);

        if (sm.staticFireTask == NULL)
        {
          sprintf(stateMsg, "Error - staticFireTask not created");
          sm.changeState(IDLE);
        }
        else
        {
          sprintf(stateMsg, "State: COUNTDOWN");
        }
        break;
      case STATIC_FIRE:
        sprintf(stateMsg, "State: STATIC_FIRE");
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
        sprintf(stateMsg, "State: ABORT");
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

