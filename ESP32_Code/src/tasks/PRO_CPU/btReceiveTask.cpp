#include "../../../include/tasks/loopTasks.h"

void btReceiveTask(void *arg)
{
  String msg;
  char msgOut[100];

  while (true)
  {
    if (btUI.isConnected())
    {
      if (btUI.available())
      {
        msg = btUI.readString();
        xQueueSend(sm.btRxQueue, (void *)&msg, 10); // TO DO error handling
      }
    }
    else
    {
      // if(sm.state != DISCONNECTED){
      if (sm.state < COUNTDOWN)
      {
        sm.changeState(DISCONNECTED);
      }
      else
      {
        sprintf(msgOut, "LOG: Disconnected in state: %d", sm.state);
        xQueueSend(sm.sdQueue, &msgOut, 0);
      }

      // disconnected sound
      beepBoop(300, 6);

      while (!btUI.isConnected())
      {
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      // connected sound
      beepBoop(300, 2);
      if (sm.state < COUNTDOWN)
      {
        sm.changeState(IDLE);
      }

      sprintf(msgOut, "State: %d", sm.state);
      xQueueSend(sm.btTxQueue, &msgOut, 10);
      //}
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}