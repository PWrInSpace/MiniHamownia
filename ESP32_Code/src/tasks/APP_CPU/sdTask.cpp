#include "../../../include/tasks/loopTasks.h"

void sdTask(void *arg)
{
  SDCard sd(&myspi, SD_CS);
  char header[] = "TIME; THRUST; PRESSURE; CONTINUITY; VALVE_1_STATE; VALVE_2_STATE; BATTERY\n";
  char data[100];
  uint16_t i = 0;

  vTaskDelay(100 / portTICK_RATE_MS);

  xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

  while (!sd.init())
  {
    digitalWrite(BUILTIN_LED, LOW);
    sprintf(data, "Sd init error!");
    xQueueSend(sm.btTxQueue, (void *)&data, 10);
    digitalWrite(BUILTIN_LED, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  digitalWrite(BUILTIN_LED, LOW);

  while (sd.fileExists("/data_test_" + String(i) + ".csv"))
  {
    i++;
  }
  String logPath = "/logs_test_" + String(i) + ".txt";
  String dataPath = "/data_test_" + String(i) + ".csv";

  sd.write(dataPath, header);
  xSemaphoreGive(sm.spiMutex);

  while (1)
  {
    while(uxQueueMessagesWaiting(sm.sdQueue) > 0)
    {
      if (xQueueReceive(sm.sdQueue, &data, portMAX_DELAY) == pdTRUE)
      {
        xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

        if (data[0] == 'L' && data[1] == 'O' && data[2] == 'G')
        { // write logs
          if (!sd.write(logPath, data))
          {
            // error handling
          }
        }
        else
        { // write data
          if (!sd.write(dataPath, data))
          {
            // error handling
          }
        }
        xSemaphoreGive(sm.spiMutex);
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}