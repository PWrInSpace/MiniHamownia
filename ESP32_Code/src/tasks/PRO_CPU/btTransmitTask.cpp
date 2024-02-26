#include "../../../include/tasks/loopTasks.h"

void btTransmitTask(void *arg)
{
  char btMsg[128];

  while (1)
  {
    xQueueReceive(sm.btTxQueue, (void *)&btMsg, portMAX_DELAY); // wait until data appear in queue
    if (btUI.isConnected())
    {
      //Serial.println(btMsg);
      btUI.println(btMsg);
    }
    else
    {
      // error
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}