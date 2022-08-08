#include "../../../include/tasks/loopTasks.h"

void btTransmitTask(void *arg)
{
  String btMsg;

  while (1)
  {
    xQueueReceive(sm.btTxQueue, (void *)&btMsg, portMAX_DELAY); // wait until data appear in queue
    if (btUI.isConnected())
    {

      btUI.println(btMsg);
    }
    else
    {
      // error
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}