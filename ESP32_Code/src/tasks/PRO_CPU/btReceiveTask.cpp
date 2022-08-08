#include "../../../include/tasks/loopTasks.h"

void btReceiveTask(void *arg)
{
  // pinMode(BUZZER, OUTPUT); //move to pinout.h
  String msg;

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
        msg = "LOG: Disconnected in state: " + String(sm.state) + "\n";
        xQueueSend(sm.sdQueue, (void *)&msg, 0);
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

      msg = "State: " + String(sm.state);
      xQueueSend(sm.btTxQueue, (void *)&msg, 10);
      //}
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}